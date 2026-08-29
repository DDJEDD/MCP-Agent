#ifndef TEMPLATE_AGENTS_H
#define TEMPLATE_AGENTS_H
#include <QString>

class template_agents
{
public:
    static bool Generate(const QString &agentName);
    static bool Rename(const QString &oldName, const QString &newName);

private:
    static QString soulTemplate(const QString &agentName);
    static QString systemPromptTemplate();
    static QString styleTemplate();
    static QString stickersTemplate();
};

#endif // TEMPLATE_AGENTS_H
