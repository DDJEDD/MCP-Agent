#include "calllog.h"

CallLog *CallLog::instance()
{
    static CallLog *s_instance = new CallLog();
    return s_instance;
}

CallLog::CallLog(QObject *parent) : QObject(parent) {}

void CallLog::record(const Entry &entryIn)
{
    Entry entry = entryIn;
    entry.id = m_nextId++;
    m_entries.append(entry);
    while (m_entries.size() > kMaxEntries)
        m_entries.removeFirst();
    emit entryAdded(entry);
}
