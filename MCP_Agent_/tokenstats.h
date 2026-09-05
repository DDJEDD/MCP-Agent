#ifndef TOKENSTATS_H
#define TOKENSTATS_H

#include <QObject>
#include <QString>

// Singleton counter for tokens spent on Gemini calls and for the ROI of
// capping conversation history before it is sent to the model (see
// TgBot::reqAgent / FileManager::GetOldMessages). Stats persist across
// restarts in config.ini under the [TokenStats] group.
class TokenStats : public QObject
{
    Q_OBJECT
public:
    struct Snapshot {
        qint64 callCount = 0;
        qint64 promptTokens = 0;
        qint64 completionTokens = 0;
        qint64 totalTokens = 0;
        qint64 baselineHistoryTokensEst = 0;
        qint64 trimmedHistoryTokensEst = 0;
        qint64 savedTokensEst = 0;
        double costSpentUsd = 0.0;
        double costSavedUsd = 0.0;
    };

    static TokenStats *instance();

    // promptTokens/completionTokens come straight from Gemini's usageMetadata
    // (real numbers). baselineHistoryTokensEst/trimmedHistoryTokensEst are
    // estimates (see estimateTokens) of how big the history block would have
    // been uncapped vs. the capped version actually sent, used to compute
    // the tokens saved by the history-length optimization.
    void recordCall(qint64 promptTokens, qint64 completionTokens,
                     qint64 baselineHistoryTokensEst, qint64 trimmedHistoryTokensEst);

    Snapshot snapshot() const { return m_snapshot; }
    QString formatSummary() const;
    void reset();

    // Rough chars/4 estimate. Only used to compare the capped vs. uncapped
    // history block relative to each other, not as a precise token count.
    static qint64 estimateTokens(const QString &text);

signals:
    void updated();

private:
    explicit TokenStats(QObject *parent = nullptr);
    void load();
    void save();

    Snapshot m_snapshot;
};

#endif // TOKENSTATS_H
