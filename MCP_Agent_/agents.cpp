#include "agents.h"
#include "filemanager.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDebug>
agents::agents(QObject *parent) : QObject(parent) {}


QString agents::getAgentPath(const QString &agentName) const {
    QString baseDir = QCoreApplication::applicationDirPath();
    return baseDir + "/Agents/" + agentName;
}

void agents::createAgent(const QString &agentName, const QString &purpose) {
     QString dirpath = getAgentPath(agentName);

    QDir agentDir = FileManager::createDirectory(dirpath);

    QStringList fileNames = {
        "system_prompt",
        "soul",
        "min_info"
    };

    for (const QString &fileName : fileNames) {
        QString content;
        content += "# Файл: " + fileName + ".md\n";
        if(fileName == "min_info"){
            content += "Назначение агента: " + purpose + "\n";
        }
        FileManager::createAndEditFile(content, agentDir, fileName);
    }

}
bool agents::deleteAgent(const QString &agentName) {
    QString dirpath = getAgentPath(agentName);

    QDir agentDir(dirpath);

    bool success = FileManager::deleteRepo(agentName,agentDir);
    return success;
}

QStringList agents::listAgents() const
{
    QDir agentsRoot(QCoreApplication::applicationDirPath() + "/Agents");
    if (!agentsRoot.exists())
        return {};
    return agentsRoot.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}
void agents::changeAgentName(const QString &oldName, const QString &newName){
    QString oldpath = getAgentPath(oldName);
    QString dirpath = getAgentPath(newName);
    QDir dir;
    if(!dir.exists(oldpath)){
        qDebug() << "папка не существует";
        return;

    }
    if(dir.exists(dirpath)){
        qDebug() << "папка с указаным именем существует";
        return;
    }
    FileManager::renameDirectory(oldpath, dirpath);


}
QString agents::getFullPrompt(const QString &agentName) const
{
    static const QStringList files = {"soul", "min_info", "system_prompt", "stickers"};
    QStringList parts;
    for (const QString &f : files) {
        const QString content = getFile(agentName, f).trimmed();
        if (!content.isEmpty())
            parts << content;
    }
    return parts.join("\n\n");
}
void agents::editFile(const QString &agentName, const QString &content, const QString &fileToEdit){
    QString dirpath = getAgentPath(agentName);
    QDir agentDir(dirpath);

    FileManager::createAndEditFile(content, agentDir, fileToEdit);
}
QString agents::getFile(const QString &agentName,  const QString &fileToGet) const{
    QString dirpath = getAgentPath(agentName);
    QDir agentDir(dirpath);

    return FileManager::getFile(fileToGet, agentDir);
}void agents::executeCall(const QString &id, const QString &callerAgentName, const QVariantMap &args, const QString &functionName, qint64 chatId, const QString &userText,const QString &role)
{
    qDebug() << "[Agents] Вызов функции:" << functionName << "ID:" << id << "От агента:" << callerAgentName << "ChatID:" << chatId;

    if (functionName == "createAgent" && role == Constants::RoleOrchestrator) {
        QString targetAgent = args.value("agentName", args.value("agent_name").toString()).toString().trimmed();
        QString purpose = args.value("purpose").toString().trimmed();

        if (targetAgent.isEmpty()) {
            qWarning() << "[Agents] Ошибка: Имя нового агента не указано в args!";
            return;
        }

        qDebug() << "[Agents] Создание агента:" << targetAgent << "Цель:" << purpose;
        createAgent(targetAgent, purpose);
    }

    else if (functionName == "deleteAgent" && role ==  Constants::RoleOrchestrator) {
        QString targetAgent = args.value("agentName", args.value("agent_name").toString()).toString().trimmed();
        if (targetAgent.isEmpty()) targetAgent = callerAgentName;

        qDebug() << "[Agents] Удаление агента:" << targetAgent;
        deleteAgent(targetAgent);
    }

    else if (functionName == "changeAgentName" && role == Constants::RoleOrchestrator) {
        QString oldName = args.value("oldName", args.value("old_name", callerAgentName).toString()).toString().trimmed();
        QString newName = args.value("newName", args.value("new_name").toString()).toString().trimmed();

        if (newName.isEmpty()) {
            qWarning() << "[Agents] Ошибка: Новое имя агента не указано!";
            return;
        }

        qDebug() << "[Agents] Переименование агента с" << oldName << "на" << newName;
        changeAgentName(oldName, newName);
    }

    else if (functionName == "editFile" && role == Constants::RoleOrchestrator) {
        QString targetAgent = args.value("agentName", args.value("agent_name", callerAgentName).toString()).toString().trimmed();
        QString fileToEdit = args.value("fileToEdit", args.value("file_to_edit").toString()).toString().trimmed();
        QString content = args.value("content").toString();

        if (targetAgent.isEmpty() || fileToEdit.isEmpty()) {
            qWarning() << "[Agents] Ошибка: Не указан целевой агент или имя файла в editFile!";
            return;
        }

        qDebug() << "[Agents] Изменение файла" << fileToEdit << "у агента:" << targetAgent;
        editFile(targetAgent, content, fileToEdit);
    }

    else if (functionName == "getFile" && role == Constants::RoleOrchestrator) {
        QString targetAgent = args.value("agentName", args.value("agent_name", callerAgentName).toString()).toString().trimmed();
        QString fileToGet = args.value("fileToGet", args.value("file_to_get").toString()).toString().trimmed();

        getFile(targetAgent, fileToGet);
    }

    else if(functionName == "reqAgent"){
        QString targetAgent = args.value("agentName", args.value("agent_name", callerAgentName).toString()).toString().trimmed();
        if (role != "orchestrator" && targetAgent != "Главный агент") {
            qWarning() << "[Agents] Ошибка безопасности: Субагент" << callerAgentName
                       << "пытается вызвать другого сабагента (" << targetAgent
                       << "). Разрешено вызывать только Главного агента!";
            return;
        }
        if (targetAgent.isEmpty() || targetAgent == callerAgentName) {
            qWarning() << "[Agents] Ошибка: Агент" << callerAgentName << "пытается вызвать сам себя или не указал цель!";
            return;
        }
        QString prompt = args.value("prompt", args.value("prompt", callerAgentName).toString()).toString().trimmed();
        QByteArray imageData = QByteArray::fromBase64(args.value("photoB64", args.value("photo_b64", "")).toString().toLatin1());
        qint64 chatID = args.value("chatId", args.value("chadId", chatId)).toLongLong();

        emit requestReqAgent(prompt, chatID, targetAgent, imageData );
    }

    if (args.contains("messages") && role == Constants::RoleOrchestrator) {
        QVariantList msgList = args.value("messages").toList();
        QList<DelayedMessage> delayedMsgs;
        QString fullAiResponse;

        for (const QVariant &item : msgList) {
            QVariantMap msgMap = item.toMap();

            QString text = msgMap.value("text").toString().trimmed();
            int delay = msgMap.value("delay", 0).toInt();


            QString stickerId;
            if (msgMap.contains("stickerId")) {
                stickerId = msgMap.value("stickerId").toString().trimmed();
            } else if (msgMap.contains("sticker")) {
                stickerId = msgMap.value("sticker").toString().trimmed();
            }
            QString photoUrl;
            if (msgMap.contains("photoUrl")) {
                photoUrl = msgMap.value("photoUrl").toString().trimmed();
            } else if (msgMap.contains("photoUrl")) {
                photoUrl = msgMap.value("photoUrl").toString().trimmed();
            }


            if (text.isEmpty() && stickerId.isEmpty()) {
                continue;
            }

            DelayedMessage dMsg;
            dMsg.text = text;
            dMsg.stickerId = stickerId;
            dMsg.delay = delay;
            dMsg.photoUrl = photoUrl;
            delayedMsgs.append(dMsg);

            if (!text.isEmpty()) {
                if (!fullAiResponse.isEmpty()) fullAiResponse += "\n";
                fullAiResponse += text;
            }
        }
        if (!delayedMsgs.isEmpty() && chatId != 0) {
            emit requestSendMessagesDelayed(chatId, delayedMsgs);
        }

        if (!fullAiResponse.isEmpty() && chatId != 0) {
            QString agentLabel = QString("[Ответ от агента: %1]").arg(callerAgentName);
            FileManager::SaveMessage(chatId, userText, fullAiResponse);
        }
    }
}
