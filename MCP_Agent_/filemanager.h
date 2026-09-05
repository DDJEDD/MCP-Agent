#ifndef MCP_H
#define MCP_H
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDir>
class FileManager
{
public:
    static void SaveMessage(qint64 userid,  const QString &userprompt,  const QString &airesp);
    static QString GetOldMessages(qint64 userid);
    // maxExchanges caps how many of the most recent user/AI exchanges are
    // included, so the prompt doesn't grow forever. Pass -1 for no cap.
    static QString GetOldMessages(qint64 userid, int maxExchanges);
    static QDir createDirectory(const QString &dirpath);
    static void createAndEditFile(const QString &content, const  QDir &dirEngine, const QString &fileName);
    static bool deleteRepo(const QString &agentName, QDir dirEngine);
    static void renameDirectory(const QString &oldpath, const QString &dirpath);
    static QString getFile(const QString &fileToGet, QDir dirEngine);
    static void loadEnvFile(const QString &filePath);
};

#endif // MCP_H
