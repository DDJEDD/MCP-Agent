#include "tokenstats.h"

#include <QSettings>
#include <algorithm>

TokenStats *TokenStats::instance()
{
    static TokenStats *s_instance = new TokenStats();
    return s_instance;
}

TokenStats::TokenStats(QObject *parent) : QObject(parent)
{
    load();
}

qint64 TokenStats::estimateTokens(const QString &text)
{
    if (text.isEmpty())
        return 0;
    return std::max<qint64>(1, text.length() / 4);
}

void TokenStats::recordCall(qint64 promptTokens, qint64 completionTokens,
                             qint64 baselineHistoryTokensEst, qint64 trimmedHistoryTokensEst)
{
    QSettings settings(QString(APP_SRC_DIR) + "/config.ini", QSettings::IniFormat);
    settings.beginGroup("TokenStats");
    const double priceIn = settings.value("priceInputPerM", 0.30).toDouble();
    const double priceOut = settings.value("priceOutputPerM", 2.50).toDouble();
    settings.endGroup();

    const qint64 savedNow = std::max<qint64>(0, baselineHistoryTokensEst - trimmedHistoryTokensEst);

    m_snapshot.callCount += 1;
    m_snapshot.promptTokens += promptTokens;
    m_snapshot.completionTokens += completionTokens;
    m_snapshot.totalTokens += promptTokens + completionTokens;
    m_snapshot.baselineHistoryTokensEst += baselineHistoryTokensEst;
    m_snapshot.trimmedHistoryTokensEst += trimmedHistoryTokensEst;
    m_snapshot.savedTokensEst += savedNow;
    m_snapshot.costSpentUsd += (promptTokens / 1000000.0) * priceIn + (completionTokens / 1000000.0) * priceOut;
    m_snapshot.costSavedUsd += (savedNow / 1000000.0) * priceIn;

    save();
    emit updated();
}

void TokenStats::reset()
{
    m_snapshot = Snapshot();
    save();
    emit updated();
}

void TokenStats::load()
{
    QSettings settings(QString(APP_SRC_DIR) + "/config.ini", QSettings::IniFormat);
    settings.beginGroup("TokenStats");
    m_snapshot.callCount = settings.value("callCount", 0).toLongLong();
    m_snapshot.promptTokens = settings.value("promptTokens", 0).toLongLong();
    m_snapshot.completionTokens = settings.value("completionTokens", 0).toLongLong();
    m_snapshot.totalTokens = settings.value("totalTokens", 0).toLongLong();
    m_snapshot.baselineHistoryTokensEst = settings.value("baselineHistoryTokensEst", 0).toLongLong();
    m_snapshot.trimmedHistoryTokensEst = settings.value("trimmedHistoryTokensEst", 0).toLongLong();
    m_snapshot.savedTokensEst = settings.value("savedTokensEst", 0).toLongLong();
    m_snapshot.costSpentUsd = settings.value("costSpentUsd", 0.0).toDouble();
    m_snapshot.costSavedUsd = settings.value("costSavedUsd", 0.0).toDouble();
    settings.endGroup();
}

void TokenStats::save()
{
    QSettings settings(QString(APP_SRC_DIR) + "/config.ini", QSettings::IniFormat);
    settings.beginGroup("TokenStats");
    settings.setValue("callCount", m_snapshot.callCount);
    settings.setValue("promptTokens", m_snapshot.promptTokens);
    settings.setValue("completionTokens", m_snapshot.completionTokens);
    settings.setValue("totalTokens", m_snapshot.totalTokens);
    settings.setValue("baselineHistoryTokensEst", m_snapshot.baselineHistoryTokensEst);
    settings.setValue("trimmedHistoryTokensEst", m_snapshot.trimmedHistoryTokensEst);
    settings.setValue("savedTokensEst", m_snapshot.savedTokensEst);
    settings.setValue("costSpentUsd", m_snapshot.costSpentUsd);
    settings.setValue("costSavedUsd", m_snapshot.costSavedUsd);
    settings.endGroup();
}

QString TokenStats::formatSummary() const
{
    const double savedPct = m_snapshot.baselineHistoryTokensEst > 0
                                 ? (100.0 * m_snapshot.savedTokensEst / m_snapshot.baselineHistoryTokensEst)
                                 : 0.0;

    return QString(
               "\U0001F4CA Статистика токенов\n"
               "Запросов к ИИ: %1\n"
               "Потрачено токенов: %2 (промпт: %3 / ответ: %4)\n"
               "Примерная стоимость: $%5\n\n"
               "♻ Экономия от ограничения истории диалога\n"
               "Сэкономлено токенов (оценка): %6 (~%7%)\n"
               "Экономия в деньгах (оценка): $%8\n\n"
               "Цены редактируются в config.ini ([TokenStats] priceInputPerM / priceOutputPerM)."
               )
        .arg(m_snapshot.callCount)
        .arg(m_snapshot.totalTokens)
        .arg(m_snapshot.promptTokens)
        .arg(m_snapshot.completionTokens)
        .arg(QString::number(m_snapshot.costSpentUsd, 'f', 4))
        .arg(m_snapshot.savedTokensEst)
        .arg(QString::number(savedPct, 'f', 1))
        .arg(QString::number(m_snapshot.costSavedUsd, 'f', 4));
}
