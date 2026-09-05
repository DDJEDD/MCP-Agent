#include "tgbot.h"
#include "filemanager.h"
#include "tokenstats.h"
#include "calllog.h"
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>
#include <optional>

// How many of the most recent user/AI exchanges get sent to Gemini as
// history. Without this cap GetOldMessages() would keep resending the
// entire conversation on every single message, growing without bound.
static constexpr int kHistoryExchangeLimit = 12;

// Recursively truncates long string values (e.g. base64 image data) before a
// JSON response is stored in CallLog, so the journal UI stays readable.
static QJsonValue redactLongStrings(const QJsonValue &value, int maxLen = 800)
{
    if (value.isString()) {
        const QString s = value.toString();
        if (s.length() > maxLen)
            return QString("%1… [обрезано, всего %2 символов]").arg(s.left(maxLen)).arg(s.length());
        return s;
    }
    if (value.isArray()) {
        QJsonArray out;
        for (const auto &v : value.toArray())
            out.append(redactLongStrings(v, maxLen));
        return out;
    }
    if (value.isObject()) {
        QJsonObject out;
        const QJsonObject obj = value.toObject();
        for (auto it = obj.begin(); it != obj.end(); ++it)
            out[it.key()] = redactLongStrings(it.value(), maxLen);
        return out;
    }
    return value;
}

static QString formatResponseForLog(const QJsonObject &response)
{
    return QString::fromUtf8(QJsonDocument(redactLongStrings(response).toObject()).toJson(QJsonDocument::Indented));
}

TgBot::TgBot(QObject *parent) : QObject(parent) {
    FileManager::loadEnvFile(".env");
    geminiKey = qgetenv("GEMINI_API_KEY");
    if (token.isEmpty()) {
        qWarning() << "ВНИМАНИЕ: GEMINI_API_KEY не найден в .env файле!";
    }
    phone = new phonenumber(this);
    m_agents = new agents(this);
    connect(m_agents, &agents::requestSendMessagesDelayed,
            this, &TgBot::sendMessagesDelayed);

    connect(m_agents, &agents::requestSendPhoto,
            this, &TgBot::sendPhoto);

    connect(m_agents, &agents::requestSendSticker,
            this, &TgBot::sendSticker);
    connect(m_agents, &agents::requestReqAgent,
            this, &TgBot::reqAgent);
    poll();

}
void TgBot::checkreqPhoto(const QJsonObject &response, qint64 chatId, const QString &prompt) {
    if (response.contains("error")) {
        qWarning() << "Gemini Photo Error:" << response["error"].toObject()["message"].toString();
        sendMessage(chatId, "Не удалось сгенерировать изображение. Попробуйте изменить запрос.");
        return;
    }

    QJsonArray candidates = response["candidates"].toArray();
    if (candidates.isEmpty()) {
        sendMessage(chatId, "ИИ не смог сгенерировать картинку.");
        return;
    }

    QJsonArray parts = candidates[0].toObject()["content"].toObject()["parts"].toArray();
    for (const QJsonValue &val : parts) {
        QJsonObject partObj = val.toObject();

        if (partObj.contains("inline_data")) {
            QString base64Data = partObj["inline_data"].toObject()["data"].toString();


            QString photoUrl = QString("data:image/png;base64,%1").arg(base64Data);

            sendPhoto(chatId, photoUrl, QString("🎨 <i>%1</i>").arg(prompt));
            return;
        }
    }

    sendMessage(chatId, "В ответе ИИ не оказалось изображения.");
}

void TgBot::downloadFile(const QString &fileId, std::function<void(const QByteArray &)> callback) {
    QString path = QString("/bot%1/getFile").arg(token);
    QJsonObject body{{"file_id", fileId}};

    requests->apiCall(this, host, path, body, {}, [this, callback](const QJsonObject &resp) {
        if (!resp.value("ok").toBool()) {
            qWarning() << "Ошибка получения файла из Telegram:" << resp;
            callback(QByteArray());
            return;
        }

        QString filePath = resp["result"].toObject()["file_path"].toString();
        if (filePath.isEmpty()) {
            qWarning() << "Telegram вернул пустой file_path";
            callback(QByteArray());
            return;
        }

        QString downloadPath = QString("/file/bot%1/%2").arg(token, filePath);

        requests->downloadFileCall(this, host, downloadPath, [callback](const QByteArray &fileData) {
            callback(fileData);
        });
    });
}
void TgBot::reqPhotoAI(const QString &userText, qint64 chatId) {
    if (geminiKey.isEmpty()) {
        sendMessage(chatId, "Ошибка конфигурации бота.");
        return;
    }

    if (userText.isEmpty()) {
        sendMessage(chatId, "Укажите описание картинки! Пример: <code>/generate неоновый город</code>");
        return;
    }

    sendTyping(chatId);

    QString aiHost = "generativelanguage.googleapis.com";
    QString aiPath = QString("/v1beta/models/gemini-2.5-flash-image:generateContent?key=%1").arg(geminiKey);

    QJsonObject body;
    QJsonObject textPart{{"text", userText}};
    QJsonObject contentsObj{{"parts", QJsonArray{textPart}}};
    body["contents"] = QJsonArray{contentsObj};

    QJsonObject generationConfig;
    generationConfig["responseModalities"] = QJsonArray{"IMAGE"};
    body["generationConfig"] = generationConfig;

    QElapsedTimer callTimer;
    callTimer.start();
    requests->apiCall(this, aiHost, aiPath, body, {}, [this, chatId, userText, callTimer](const QJsonObject &response) {
        const bool hasError = response.contains("error");
        const bool hasImage = !response.value("candidates").toArray().isEmpty();

        CallLog::Entry logEntry;
        logEntry.timestamp = QDateTime::currentDateTime();
        logEntry.chatId = chatId;
        logEntry.kind = "Изображение";
        logEntry.agentName = "Генератор изображений";
        logEntry.userText = userText;
        logEntry.aiText = hasError
            ? response.value("error").toObject().value("message").toString()
            : (hasImage ? "Изображение сгенерировано" : "Ответ без изображения");
        logEntry.fullPrompt = userText;
        logEntry.rawResponse = formatResponseForLog(response);
        logEntry.durationMs = callTimer.elapsed();
        logEntry.success = !hasError && hasImage;
        CallLog::instance()->record(logEntry);

        checkreqPhoto(response, chatId, userText);
    });
}
void TgBot::processPhotoMessage(const QString &fileId, const QString &text, qint64 chatId) {
    qDebug() << "Получено изображение:" << fileId << "Чат:" << chatId << "Подпись:" << text;

    downloadFile(fileId, [this, text, chatId](const QByteArray &imageData) {
        if (imageData.isEmpty()) {
            qWarning() << "Не удалось загрузить изображение";
            sendMessage(chatId, "Ошибка при обработке изображения.");
            return;
        }

        reqAgent(text.isEmpty() ? "Что на этой картинке?" : text, chatId, imageData);
    });
}
void TgBot::checkreq(const QJsonObject &response, qint64 chatId, const QString &text, const QMap<QString, QString> &nums) {

    QJsonArray candidates = response["candidates"].toArray();
    if (!candidates.isEmpty()) {
        QJsonArray parts = candidates[0].toObject()["content"].toObject()["parts"].toArray();
        if (!parts.isEmpty()) {
            QString aiText = parts[0].toObject()["text"].toString();
            qDebug().noquote() << "Ответ от Gemini (текст):" << aiText;
        }
    }

    auto optCalls = JSONParser::parse(response, nums, *phone);
    if (!optCalls.has_value() || optCalls->isEmpty()) {
        qWarning() << "Не удалось распарсить ответ от Gemini или стек вызовов пуст.";
        sendMessage(chatId, "Ошибка при обработке ответа от ИИ.");
        return;
    }

    const QList<AgentCall> &calls = *optCalls;
    qDebug() << "[TgBot] Получено вызовов сабагентов:" << calls.size();

    for (const AgentCall &call : calls) {
        qDebug() << "  -> Запуск функции:" << call.functionName
                 << "для агента:" << call.agentName
                 << "ID:" << call.id;


        this->m_agents->executeCall(call.id, call.agentName, call.args, call.functionName, chatId, text, call.role);
    }
}

void TgBot::reqAgent(const QString &userText, qint64 chatId,const QString &agentName, const QByteArray &imageData) {


    if (geminiKey.isEmpty()) {
        qWarning() << "GEMINI_API_KEY не установлен!";
        sendMessage(chatId, "Ошибка конфигурации бота.");
        return;
    }
    QString aiHost = "generativelanguage.googleapis.com";
    QString aiPath = QString("/v1beta/models/gemini-3.6-flash:generateContent?key=%1").arg(geminiKey);

    QStringList agentNames = m_agents->listAgents();
    QString agentsListStr = "Доступные субагенты в системе: " + agentNames.join(", ");

    const QString fullHistory = FileManager::GetOldMessages(chatId);
    const QString trimmedHistory = FileManager::GetOldMessages(chatId, kHistoryExchangeLimit);

    QString final = m_agents->getFullPrompt(agentName) + "\nHISTORY:"  + "\n\n[СИСТЕМНАЯ СПРАВКА]\n" + agentsListStr + trimmedHistory;
    textwithoutnum finaluserText = phone->HideNumbers(userText);
    QString fullContextText = final + "\n" + finaluserText.usertext;

    qDebug() << fullContextText;
    QJsonObject body;
    QJsonArray partsArray;
    QJsonObject textPart;
    textPart["text"] = fullContextText;
    partsArray.append(textPart);
    sendTyping(chatId);
    if (!imageData.isEmpty()) {
        QJsonObject imagePart;
        QJsonObject inlineData;

        inlineData["mime_type"] = "image/jpeg";
        inlineData["data"] = QString(imageData.toBase64());
        imagePart["inline_data"] = inlineData;
        partsArray.append(imagePart);
    }

    QJsonObject contentsObj;
    contentsObj["parts"] = partsArray;

    QJsonArray contentsArray;
    contentsArray.append(contentsObj);
    body["contents"] = contentsArray;

    QJsonObject generationConfig;
    generationConfig["responseMimeType"] = "application/json";
    generationConfig["maxOutputTokens"] = 4096;
    body["generationConfig"] = generationConfig;


    QMap<QString, QString> nums = finaluserText.numbers;
    QElapsedTimer callTimer;
    callTimer.start();
    requests->apiCall(this, aiHost, aiPath, body, {}, [this, chatId, userText, nums, fullHistory, trimmedHistory, fullContextText, agentName, callTimer](const QJsonObject &response) {
        const QJsonObject usage = response.value("usageMetadata").toObject();
        const qint64 promptTokens = usage.value("promptTokenCount").toVariant().toLongLong();
        const qint64 completionTokens = usage.value("candidatesTokenCount").toVariant().toLongLong();
        const qint64 baselineEst = TokenStats::estimateTokens(fullHistory);
        const qint64 trimmedEst = TokenStats::estimateTokens(trimmedHistory);
        if (!usage.isEmpty()) {
            TokenStats::instance()->recordCall(promptTokens, completionTokens, baselineEst, trimmedEst);
        }

        QString aiText;
        bool success = !response.contains("error");
        if (!success) {
            aiText = response.value("error").toObject().value("message").toString();
        } else {
            const QJsonArray candidates = response.value("candidates").toArray();
            if (!candidates.isEmpty()) {
                const QJsonArray parts = candidates.first().toObject().value("content").toObject().value("parts").toArray();
                if (!parts.isEmpty())
                    aiText = parts.first().toObject().value("text").toString();
            }
            if (aiText.isEmpty())
                success = false;
        }

        CallLog::Entry logEntry;
        logEntry.timestamp = QDateTime::currentDateTime();
        logEntry.chatId = chatId;
        logEntry.kind = "Диалог";
        logEntry.agentName = agentName;
        logEntry.userText = userText;
        logEntry.aiText = aiText;
        logEntry.fullPrompt = fullContextText;
        logEntry.rawResponse = formatResponseForLog(response);
        logEntry.promptTokens = promptTokens;
        logEntry.completionTokens = completionTokens;
        logEntry.durationMs = callTimer.elapsed();
        logEntry.success = success;
        logEntry.baselineHistoryTokensEst = baselineEst;
        logEntry.trimmedHistoryTokensEst = trimmedEst;
        CallLog::instance()->record(logEntry);

        checkreq(response, chatId, userText, nums);
    });
}

void TgBot::poll() {
    const QString path = QString("/bot%1/getUpdates").arg(token);
    QJsonObject body{{"timeout", 30}, {"offset", offset}};

    requests->apiCall(this, host, path, body, {}, [this](const QJsonObject &resp) {
        if (!resp.value("ok").toBool()) {
            qWarning() << "getUpdates вернул ошибку:" << resp;
            poll();
            return;
        }

        for (const auto &v : resp.value("result").toArray()) {
            const auto update = v.toObject();
            offset = update["update_id"].toInteger() + 1;

            const auto message = update["message"].toObject();
            if (message.isEmpty())
                continue;

            const qint64 chatId = message["chat"].toObject()["id"].toInteger();
            const QString text = message["caption"].toString().isEmpty()
                                     ? message["text"].toString()
                                     : message["caption"].toString();

            if (message.contains("photo") && message["photo"].isArray()) {
                QJsonArray photoArray = message["photo"].toArray();
                if (!photoArray.isEmpty()) {
                    QJsonObject largestPhoto = photoArray.last().toObject();
                    QString fileId = largestPhoto["file_id"].toString();
                    processPhotoMessage(fileId, text, chatId);
                    continue;
                }
            }

            if (chatId != 0 && !text.isEmpty())
                if (text.startsWith("/image")) {
                    QString promptText = text.mid(6).trimmed();
                    reqPhotoAI(promptText, chatId);
                } else if (text == "/start") {
                    sendMessage(chatId, "На связи Олег Сигмов, senior AI-ассистент по разработке, DevOps и системной инженерии из Sigmov LTD. А ещё у нас на вооружении появилась новая фича — команда /generate. Напиши её, опиши задачу, и я сгенерирую тебе сочный арт, техническую схему или архитектурный концепт.");
                } else if (text == "/stats") {
                    sendMessage(chatId, TokenStats::instance()->formatSummary());
                } else {

                    reqAgent(text, chatId, "Главный агент");
                }
        }
        QTimer::singleShot(0, this, &TgBot::poll);
    });
}

void TgBot::sendMessage(qint64 chatId, const QString &text) {
    const QString path = QString("/bot%1/sendMessage").arg(token);
    QJsonObject body{{"chat_id", chatId}, {"text", text}, {"parse_mode", "HTML"}};
    requests->apiCall(this, host, path, body, {}, [](const QJsonObject &) {});
}

void TgBot::sendAnimation(qint64 chatId, const QString &animationUrl, const QString &caption) {
    const QString path = QString("/bot%1/sendAnimation").arg(token);
    QJsonObject body{
        {"chat_id", chatId},
        {"animation", animationUrl},
        {"caption", caption},
        {"parse_mode", "HTML"}
    };

    requests->apiCall(this, host, path, body, {}, [this, chatId, caption](const QJsonObject &resp) {
        if (!resp.value("ok").toBool()) {
            qWarning() << "sendAnimation failed:" << resp["description"].toString();
            qWarning() << "Falling back to sending text message only...";

            sendMessage(chatId, caption);
        } else {
            qDebug() << "Animation sent successfully!";
        }
    });
}
void TgBot::sendPhoto(qint64 chatId, const QString &photoUrl, const QString &caption)
{
    const QString path = QString("/bot%1/sendPhoto").arg(token);

    QJsonObject body{
        {"chat_id", chatId},
        {"photo", photoUrl},
        {"caption", caption},
        {"parse_mode", "HTML"}
    };

    requests->apiCall(this,host,path,body,{},[this, chatId, caption](const QJsonObject &resp) {

        if (!resp.value("ok").toBool()) {
            qWarning() << "sendPhoto failed:"
                       << resp["description"].toString();

            qWarning() << "Falling back to sending text message only...";

            sendMessage(chatId, caption);
        } else {
            qDebug() << "Photo sent successfully!";
        }
    }
                      );
}
void TgBot::sendSticker(qint64 chatId, const QString &stickerId)
{
    const QString path = QString("/bot%1/sendSticker").arg(token);

    QJsonObject body{
        {"chat_id", chatId},
        {"sticker", stickerId}
    };

    requests->apiCall( this, host, path,body,{}, [](const QJsonObject &resp) {
        if (!resp.value("ok").toBool()) {
            qWarning() << "sendSticker failed:"
                       << resp["description"].toString();
        }
    }
                      );
}
void TgBot::sendTyping(qint64 chatId) {
    const QString path = QString("/bot%1/sendChatAction").arg(token);

    QJsonObject body{
        {"chat_id", chatId},
        {"action", "typing"}
    };

    requests->apiCall(this, host, path, body, {}, [](const QJsonObject &) {});
}void TgBot::sendMessagesDelayed(qint64 chatId, const QList<DelayedMessage> &messages)
{
    if (messages.isEmpty()) return;

    auto index = std::make_shared<int>(0);
    auto *timer = new QTimer(this);
    timer->setSingleShot(true);

    auto sendNext = [this, chatId, messages, index, timer]() mutable {
        if (*index >= messages.size()) {
            timer->deleteLater();
            return;
        }

        const DelayedMessage &msg = messages[*index];

        if (!msg.stickerId.isEmpty()) {
            qDebug() << "[TgBot] Отправляем стикер:" << msg.stickerId;
            sendSticker(chatId, msg.stickerId);
        }
        else if (!msg.photoUrl.isEmpty()) {
            qDebug() << "[TgBot] Отправляем фото:" << msg.photoUrl;
            sendPhoto(chatId, msg.photoUrl,msg.text);
        }
        else if (!msg.text.isEmpty()) {
            qDebug() << "[TgBot] Отправляем текст:" << msg.text;
            sendMessage(chatId, msg.text);
        }

        (*index)++;

        if (*index >= messages.size()) {
            timer->deleteLater();
            return;
        }

        int nextDelayMs = messages[*index].delay > 0 ? messages[*index].delay * 1000 : 0;
        timer->start(nextDelayMs);
    };

    connect(timer, &QTimer::timeout, this, sendNext);
    sendNext();
}
