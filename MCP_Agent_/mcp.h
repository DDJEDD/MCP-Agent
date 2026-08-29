#ifndef MCP_H
#define MCP_H
#include <QObject>
#include <QFile>
#include <QTextStream>
class MCP
{
public:
    static QString GetPrompt(QString PromptPath);
    static void SaveMessage(qint64 userid, QString userprompt, QString airesp);
    static QString GetOldMessages(qint64 userid);
};

#endif // MCP_H
