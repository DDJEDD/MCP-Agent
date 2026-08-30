#include "tgbot.h"
#include "mcp.h"
#include "template_agents.h"
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QTimer>
#include <QSettings>


TgBot::TgBot(QObject *parent) : QObject(parent) {
    QSettings settings(QString(APP_SRC_DIR) + "/config.ini", QSettings::IniFormat);
    token = settings.value("telegram_token", "8979215541:AAGMuBOHM81rE3y8R-iK7wsFuAGfTy4ckXI").toString();
    geminiKey = settings.value("gemini_api_key", QString(qgetenv("GEMINI_KEY"))).toString();

    template_agents::Generate("main");
    poll();
    phone = new phonenumber(this);
}

void TgBot::applyCredentials(const QString &botToken, const QString &geminiApiKey) {
    token = botToken;
    geminiKey = geminiApiKey;
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
static QJsonDocument extractFirstJsonObject(const QString &rawText, QJsonParseError *error) {
    int depth = 0;
    bool inString = false, escaped = false;
    int endPos = -1;

    for (int i = 0; i < rawText.length(); ++i) {
        QChar c = rawText[i];
        if (escaped) { escaped = false; continue; }
        if (c == '\\' && inString) { escaped = true; continue; }
        if (c == '"') { inString = !inString; continue; }
        if (inString) continue;

        if (c == '{') depth++;
        else if (c == '}') {
            if (--depth == 0) { endPos = i; break; }
        }
    }

    if (endPos != -1) {
        return QJsonDocument::fromJson(rawText.left(endPos + 1).trimmed().toUtf8(), error);
    }
    return QJsonDocument();
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

    requests->apiCall(this, aiHost, aiPath, body, {}, [this, chatId, userText](const QJsonObject &response) {
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

        reqAI(text.isEmpty() ? "Что на этой картинке?" : text, chatId, imageData);
    });
}
void TgBot::checkreq(const QJsonObject &response, qint64 chatId, const QString &text, const QMap<QString, QString> &nums) {
    qDebug() << "Ответ от Gemini:" << response;

    if (response.contains("error")) {
        QJsonObject err = response["error"].toObject();
        qWarning() << "Gemini API вернул ошибку:" << err["message"].toString();
        sendMessage(chatId, "Ошибка при обращении к ИИ. Попробуйте позже.");
        return;
    }

    QJsonArray candidates = response["candidates"].toArray();
    if (candidates.isEmpty()) {
        qWarning() << "В ответе Gemini нет 'candidates'!" << response;
        sendMessage(chatId, "ИИ не смог сформировать ответ. Попробуйте переформулировать вопрос.");
        return;
    }

    QJsonObject firstCandidate = candidates[0].toObject();
    QJsonArray parts = firstCandidate["content"].toObject()["parts"].toArray();
    if (parts.isEmpty()) {
        qWarning() << "В ответе Gemini пустой 'parts'!";
        sendMessage(chatId, "ИИ вернул пустой ответ. Попробуйте ещё раз.");
        return;
    }

    QString rawAiText = parts[0].toObject()["text"].toString().trimmed();
    qDebug() << "Сырой текст от Gemini:" << rawAiText;

    if (rawAiText.startsWith("```")) {
        rawAiText.remove(QRegularExpression("^```(?:json)?\\s*"));
        rawAiText.remove(QRegularExpression("\\s*```$"));
        rawAiText = rawAiText.trimmed();
    }

    if (rawAiText.isEmpty()) {
        qWarning() << "После очистки текст от Gemini оказался пустым!";
        sendMessage(chatId, "ИИ вернул пустой ответ. Попробуйте ещё раз.");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument aiJsonDoc = QJsonDocument::fromJson(rawAiText.toUtf8(), &parseError);

    if (parseError.error == QJsonParseError::GarbageAtEnd) {

        aiJsonDoc = extractFirstJsonObject(rawAiText, &parseError);
        if (aiJsonDoc.isObject()) {
            parseError.error = QJsonParseError::NoError;
        }
    }

    if (parseError.error != QJsonParseError::NoError || !aiJsonDoc.isObject()) {
        qWarning() << "JSON ERROR:" << parseError.errorString() << "OFFSET:" << parseError.offset;
        qWarning().noquote() << rawAiText;
        sendMessage(chatId, "ИИ вернул некорректный JSON.");
        return;
    }

    QJsonObject aiJsonObj = aiJsonDoc.object();
    QList<DelayedMessage> messages;
    QJsonArray messagesArray = aiJsonObj["messages"].toArray();

    for (const QJsonValue &value : messagesArray) {
        QJsonObject messageObj = value.toObject();

        QString messageText =
            messageObj["text"].toString().trimmed();

        int delay =
            messageObj["delay"].toInt();

        if (messageText.isEmpty())
            continue;


        messageText = phone->restoreNumbers(messageText, nums);


        delay = qBound(0, delay, 5000);

        messages.append({
            messageText,
            delay
        });
    }

    if (messages.isEmpty()) {
        qWarning() << "Gemini не вернул messages!";
        sendMessage(chatId, "ИИ вернул пустой ответ.");
        return;
    }

    QString gifUrl = aiJsonObj["gif"].toString().trimmed();
    QString photoUrl = aiJsonObj["photo"].toString().trimmed();
    QString stickerUrl = aiJsonObj["sticker"].toString().trimmed();
    QString fullAiText;

    for (const auto &msg : messages) {
        if (!fullAiText.isEmpty())
            fullAiText += "\n";

        fullAiText += msg.text;
    }

    MCP::SaveMessage(chatId, text, fullAiText);
    if (!photoUrl.isEmpty()) {
        sendPhoto(chatId, photoUrl, fullAiText);
    }else if (!gifUrl.isEmpty()) {
        sendAnimation(chatId, gifUrl, fullAiText);

    } else if(!stickerUrl.isEmpty()) {
        sendSticker(chatId, stickerUrl);
        QTimer::singleShot(2000, this, [this, chatId, messages]() {
            sendMessagesDelayed(chatId, messages);
        });
    }

    else {
        sendMessagesDelayed(chatId, messages);
    }

}

void TgBot::reqAI(const QString &userText, qint64 chatId, const QByteArray &imageData) {


    if (geminiKey.isEmpty()) {
        qWarning() << "GEMINI_API_KEY не установлен!";
        sendMessage(chatId, "Ошибка конфигурации бота.");
        return;
    }
    QString aiHost = "generativelanguage.googleapis.com";
    QString aiPath = QString("/v1beta/models/gemini-3.5-flash:generateContent?key=%1").arg(geminiKey);

    QString final = MCP::BuildSystemPrompt("main") + "\nHISTORY:" + MCP::GetOldMessages(chatId);
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
    requests->apiCall(this, aiHost, aiPath, body, {}, [this, chatId, userText,nums](const QJsonObject &response) {
        checkreq(response, chatId, userText, nums);
    });
}

void TgBot::handleUiMessage(const QString &agentId, const QString &promptName, const QString &text) {
    if (geminiKey.isEmpty()) {
        emit uiReplyReady(agentId, "Ошибка конфигурации бота: не задан GEMINI_KEY.");
        return;
    }

    QString aiHost = "generativelanguage.googleapis.com";
    QString aiPath = QString("/v1beta/models/gemini-3.5-flash:generateContent?key=%1").arg(geminiKey);

    QString fullContextText = MCP::BuildSystemPrompt(promptName) + "\n" + text;

    QJsonObject textPart{{"text", fullContextText}};
    QJsonArray partsArray{textPart};
    QJsonObject contentsObj{{"parts", partsArray}};
    QJsonArray contentsArray{contentsObj};

    QJsonObject body;
    body["contents"] = contentsArray;
    body["generationConfig"] = QJsonObject{{"responseMimeType", "application/json"}, {"maxOutputTokens", 4096}};

    Requests::apiCall(this, aiHost, aiPath, body, {}, [this, agentId](const QJsonObject &response) {
        emit uiReplyReady(agentId, extractAgentText(response));
    });
}

QString TgBot::extractAgentText(const QJsonObject &response) {
    if (response.contains("error"))
        return "Ошибка при обращении к ИИ. Попробуйте позже.";

    QJsonArray candidates = response["candidates"].toArray();
    if (candidates.isEmpty())
        return "ИИ не смог сформировать ответ.";

    QJsonObject firstCandidate = candidates[0].toObject();
    QJsonArray parts = firstCandidate["content"].toObject()["parts"].toArray();
    if (parts.isEmpty())
        return "ИИ вернул пустой ответ.";

    QString rawAiText = parts[0].toObject()["text"].toString().trimmed();
    if (rawAiText.startsWith("```")) {
        rawAiText.remove(QRegularExpression("^```(?:json)?\\s*"));
        rawAiText.remove(QRegularExpression("\\s*```$"));
        rawAiText = rawAiText.trimmed();
    }

    QJsonParseError parseError;
    QJsonDocument aiJsonDoc = QJsonDocument::fromJson(rawAiText.toUtf8(), &parseError);
    if (parseError.error == QJsonParseError::GarbageAtEnd) {
        aiJsonDoc = extractFirstJsonObject(rawAiText, &parseError);
        if (aiJsonDoc.isObject())
            parseError.error = QJsonParseError::NoError;
    }

    if (parseError.error != QJsonParseError::NoError || !aiJsonDoc.isObject())
        return "ИИ вернул некорректный JSON.";

    QJsonArray messagesArray = aiJsonDoc.object()["messages"].toArray();
    QString fullAiText;
    for (const QJsonValue &value : messagesArray) {
        QString messageText = value.toObject()["text"].toString().trimmed();
        if (messageText.isEmpty())
            continue;
        if (!fullAiText.isEmpty())
            fullAiText += "\n";
        fullAiText += messageText;
    }

    return fullAiText.isEmpty() ? "ИИ вернул пустой ответ." : fullAiText;
}

void TgBot::poll() {
    const QString path = QString("/bot%1/getUpdates").arg(token);
    QJsonObject body{{"timeout", 30}, {"offset", offset}};

    requests->apiCall(this, host, path, body, {}, [this](const QJsonObject &resp) {
        if (!resp.value("ok").toBool()) {
            qWarning() << "getUpdates вернул ошибку:" << resp;
            QTimer::singleShot(3000, this, &TgBot::poll);
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
                    QString promptText = text.mid(9).trimmed();
                    reqPhotoAI(promptText, chatId);
                } else if (text == "/start") {
                    sendMessage(chatId, "На связи Олег Сигмов, senior AI-ассистент по разработке, DevOps и системной инженерии из Sigmov LTD. А ещё у нас на вооружении появилась новая фича — команда /generate. Напиши её, опиши задачу, и я сгенерирую тебе сочный арт, техническую схему или архитектурный концепт.");
                } else {

                    reqAI(text, chatId);
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
}
void TgBot::sendMessagesDelayed(qint64 chatId, const QList<DelayedMessage> &messages)
{
    if (messages.isEmpty())
        return;


    auto index = std::make_shared<int>(0);
    auto *timer = new QTimer(this);

    timer->setSingleShot(true);


    auto sendNext = [this, chatId, messages, index, timer]() mutable {

        if (*index >= messages.size()) {
            timer->deleteLater();
            return;
        }

        const DelayedMessage &msg = messages[*index];
        sendMessage(chatId, msg.text);

        (*index)++;

        if (*index >= messages.size()) {
            timer->deleteLater();
            return;
        }

        timer->start(messages[*index].delay);
    };

    connect(timer, &QTimer::timeout, this, sendNext);

    sendNext();
}