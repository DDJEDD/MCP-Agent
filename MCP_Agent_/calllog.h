#ifndef CALLLOG_H
#define CALLLOG_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>

// Singleton journal of every request/response exchanged with Gemini (chat
// turns and image generations). Kept in memory only (last kMaxEntries),
// used to drive the "Аналитика и журнал" page in MainWindow.
class CallLog : public QObject
{
    Q_OBJECT
public:
    struct Entry {
        qint64 id = 0;
        QDateTime timestamp;
        qint64 chatId = 0;
        QString kind;        // "Диалог" / "Изображение"
        QString agentName;
        QString userText;    // what the user sent
        QString aiText;      // extracted AI reply, or error message
        QString fullPrompt;  // exact text sent to the model
        QString rawResponse; // raw JSON response, pretty-printed, long fields redacted
        qint64 promptTokens = 0;
        qint64 completionTokens = 0;
        qint64 durationMs = 0;
        bool success = true;
        qint64 baselineHistoryTokensEst = 0;
        qint64 trimmedHistoryTokensEst = 0;
    };

    static constexpr int kMaxEntries = 300;

    static CallLog *instance();

    void record(const Entry &entry);
    QList<Entry> entries() const { return m_entries; } // oldest first

signals:
    void entryAdded(const CallLog::Entry &entry);

private:
    explicit CallLog(QObject *parent = nullptr);

    QList<Entry> m_entries;
    qint64 m_nextId = 1;
};

#endif // CALLLOG_H
