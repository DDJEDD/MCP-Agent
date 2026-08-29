#include "mcp.h"
#include <QJsonDocument>
#include <QDebug>
#include <QJsonObject>


QString MCP::GetPrompt(QString promptPath){
    QFile promptfile(promptPath + ".md");
    if (!promptfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file for reading:" << promptfile.errorString();
        return "";
    }
    QTextStream in(&promptfile);
    QString prompt;
    while(!in.atEnd()){
        prompt.append(in.readLine());

    }
    promptfile.close();
    return prompt;
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