#include "mcp.h"
#include <QJsonDocument>
#include <QDebug>
#include <QJsonObject>
#include <QDir>


QString MCP::GetPrompt(const QString &agentName, const QString &part){
    QFile partFile(QString(APP_SRC_DIR) + QString("/agents/%1/%2.md").arg(agentName, part));
    if (!partFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file for reading:" << partFile.errorString();
        return "";
    }
    QTextStream in(&partFile);
    QString content = in.readAll();
    partFile.close();
    return content;
}

void MCP::SavePrompt(const QString &agentName, const QString &part, const QString &content){
    QDir dir;
    dir.mkpath(QString(APP_SRC_DIR) + QString("/agents/%1").arg(agentName));

    QFile partFile(QString(APP_SRC_DIR) + QString("/agents/%1/%2.md").arg(agentName, part));
    if (!partFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Could not open file for writing:" << partFile.errorString();
        return;
    }
    QTextStream out(&partFile);
    out << content;
    partFile.close();
}

QString MCP::BuildSystemPrompt(const QString &agentName){
    static const QStringList parts = {"soul", "system_prompt", "style", "stickers"};
    QString result;
    for (const QString &part : parts) {
        QString chunk = GetPrompt(agentName, part);
        if (!chunk.isEmpty())
            result += chunk + "\n";
    }
    return result;
}

void MCP::SaveMessage(qint64 userid, QString userprompt, QString airesp){
    QFile userfile(QString::number(userid) +".jsonl");
    if (!userfile.open(QIODevice::Append | QIODevice::Text)) {
        qDebug() << "Could not open file for writing:" << userfile.errorString();
        return;
    }
    QJsonObject obj;
    obj["user_prompt"] = userprompt;
    obj["ai_resp"] = airesp;
    QDateTime dateTime = QDateTime::currentDateTime();
    QString result = dateTime.toString("yyyy::MM::dd::hh::mm::ss");
    obj["datetime"] = result;
    QJsonDocument json(obj);
    userfile.write(json.toJson(QJsonDocument::Compact) + "\n");

    userfile.close();

}

QString MCP::GetOldMessages(qint64 userid){
    QFile userfile(QString::number(userid) +".jsonl");
    if (!userfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file for reading:" << userfile.errorString();
        return "";
    }
    QTextStream in(&userfile);
    QString prompt;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8());
        if (doc.isObject()) {
            QJsonObject logObj = doc.object();
            prompt.append("User: " + logObj["user_prompt"].toString() + "\n");
            prompt.append("AI: " + logObj["ai_resp"].toString() + "\n\n");

        }
    }
    userfile.close();
    return prompt;
}