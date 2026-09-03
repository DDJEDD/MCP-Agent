#ifndef AGENTS_H
#define AGENTS_H

#include <QString>
#include <QObject>
#include "jsonparser.h"

namespace Constants {
inline constexpr QStringView RoleOrchestrator = u"orchestrator";
}
class agents : public QObject
{
    Q_OBJECT

public:
    explicit agents(QObject *parent = nullptr);

    void createAgent(const QString &name, const QString &purpose);
    bool deleteAgent(const QString &name);
    QString getAgentPath(const QString &agentName) const;
    void changeAgentName(const QString &oldName,  const QString &newName);
    void editFile(const QString &agentName, const QString &content, const QString &fileToEdit);
    QString getFile(const QString &agentName,  const QString &fileToGet) const;
    QString getFullPrompt(const QString &agentName) const;
    QStringList listAgents() const;
    void executeCall(const QString &id, const QString &agentName, const QVariantMap &args, const QString &functionName, qint64 chatId, const QString &userText, const QString &role);
signals:
    void requestSendMessagesDelayed(qint64 chatId, const QList<DelayedMessage> &messages);

    void requestSendPhoto(qint64 chatId, const QString &photoUrl, const QString &caption);
    void requestSendSticker(qint64 chatId, const QString &stickerId);
    void requestReqAgent(const QString &userText, qint64 chatId,const QString &agentName, const QByteArray &imageData = QByteArray());
};

#endif // AGENTS_H
