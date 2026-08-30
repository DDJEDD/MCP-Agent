#ifndef MCP_H
#define MCP_H
#include <QObject>
#include <QFile>
#include <QTextStream>
class MCP
{
public:
    static QString GetPrompt(const QString &agentName, const QString &part);
    static void SavePrompt(const QString &agentName, const QString &part, const QString &content);
    static QString BuildSystemPrompt(const QString &agentName);
    static void SaveMessage(qint64 userid, QString userprompt, QString airesp);
    static QString GetOldMessages(qint64 userid);
};

#endif // MCP_H
