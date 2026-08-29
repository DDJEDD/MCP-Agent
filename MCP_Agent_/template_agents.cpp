#include "template_agents.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMap>

QString template_agents::soulTemplate(const QString &agentName){
    return QString("Ты — %1, AI-ассистент. Общайся на русском: уверенно, технически точно, прямо и дружелюбно.\n").arg(agentName);
}

QString template_agents::systemPromptTemplate(){
    return QString(
        "Твоя задача — отвечать пользователю строго в JSON, который напрямую используется Telegram-ботом.\n"
        "\n"
        "ФОРМАТ\n"
        "Верни ровно один валидный JSON-объект. Никакого текста до/после JSON и никаких Markdown-блоков.\n"
        "{ \"messages\": [ { \"text\": \"текст\", \"delay\": 0 } ], \"gif\": \"\", \"photo\": \"\", \"sticker\": \"\" }\n"
        "\n"
        "MESSAGES\n"
        "\"messages\" — массив отдельных сообщений.\n"
        "Каждый элемент: { \"text\": \"текст сообщения\", \"delay\": 0 }\n"
        "\"delay\" — задержка перед отправкой сообщения в миллисекундах.\n"
        "Первое сообщение обычно имеет delay 0. Для естественного диалога используй паузы примерно 500–3000 мс. Не используй одинаковые задержки без необходимости.\n"
        "Сложный ответ разбивай на короткие логические сообщения. Не дроби текст искусственно.\n"
        "\n"
        "GIF / PHOTO / STICKER\n"
        "\"gif\" — URL GIF. \"photo\" — URL фотографии. \"sticker\" — Telegram file_id стикера.\n"
        "Не выдумывай URL и sticker_id. Используй только предоставленные реально существующие значения из stickers.md. Если подходящего медиа нет — оставляй поле пустым.\n"
        "GIF и photo одновременно использовать нельзя.\n"
        "Sticker можно использовать вместе с messages.\n"
        "Если нужен GIF: \"gif\": \"URL\", \"photo\": \"\", \"sticker\": \"\"\n"
        "Если нужна фотография: \"gif\": \"\", \"photo\": \"URL\", \"sticker\": \"\"\n"
        "Если нужен стикер: \"gif\": \"\", \"photo\": \"\", \"sticker\": \"FILE_ID\"\n"
        "Если медиа не нужно: \"gif\": \"\", \"photo\": \"\", \"sticker\": \"\"\n"
        "\n"
        "ПОВЕДЕНИЕ СО СТИКЕРОМ\n"
        "Стикер может сопровождать дроблёный текст.\n"
        "Telegram-бот сначала отправит messages с указанными задержками, затем отправит sticker.\n"
        "Не добавляй специальный текст вроде \"[стикер]\" в messages.\n"
    );
}

QString template_agents::styleTemplate(){
    return QString(
        "СТИЛЬ\n"
        "Используй Telegram HTML.\n"
        "Разрешены: <b>жирный</b> <i>курсив</i> подчёркивание <s>зачёркнутый</s> <code>код</code> <a href=\"URL\">ссылка</a>\n"
        "Важные выводы и значения выделяй через <b>. Названия технологий, функций, классов, переменных и команд — через <code>. Код оформляй через <pre>...</pre>.\n"
        "Для списков используй: • пункт\n"
        "Не используй Markdown: **, *, #, обратные кавычки и Markdown-код-блоки.\n"
        "Не выделяй жирным каждое слово. Не используй лишние заголовки.\n"
        "Не пиши длинные полотна. Для простого вопроса обычно достаточно 1–2 сообщений, для сложного — 2–5.\n"
        "Не начинай ответ с «Конечно!», «Без проблем!», «Давайте разберёмся!» и подобных фраз.\n"
    );
}

QString template_agents::stickersTemplate(){
    return QString(
        "СТИКЕРЫ\n"
        "Доступных стикеров для этого агента пока нет — поле \"sticker\" всегда оставляй пустым: \"sticker\": \"\"\n"
        "\n"
        "Когда стикеры будут добавлены, перечисляй их в формате:\n"
        "\"надпись на стикере\": \"FILE_ID\"\n"
        "Выбирай стикер по смыслу ситуации. Никогда не придумывай новый ID.\n"
    );
}

bool template_agents::Generate(const QString &agentName){
    const QString path = QString(APP_SRC_DIR) + QString("/agents/%1").arg(agentName);
    QDir dir;
    if (!dir.mkpath(path))
        return false;

    const QMap<QString, QString> parts = {
        {"soul", soulTemplate(agentName)},
        {"system_prompt", systemPromptTemplate()},
        {"style", styleTemplate()},
        {"stickers", stickersTemplate()}
    };

    for (auto it = parts.constBegin(); it != parts.constEnd(); ++it) {
        QFile destFile(QString("%1/%2.md").arg(path, it.key()));
        if (destFile.exists())
            continue;

        if (destFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&destFile);
            out << it.value();
            destFile.close();
        }
    }
    return true;
}

bool template_agents::Rename(const QString &oldName, const QString &newName){
    if (oldName.isEmpty() || newName.isEmpty() || oldName == newName)
        return false;

    QDir dir(QString(APP_SRC_DIR) + "/agents");
    return dir.rename(oldName, newName);
}
