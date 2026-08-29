#ifndef PHONENUMBER_H
#define PHONENUMBER_H
#include <QString>
#include <QMap>
#include <QObject>
struct textwithoutnum{
    QString usertext;
    QMap<QString, QString> numbers;
};

class phonenumber:public QObject
{

public:
    explicit phonenumber(QObject *parent = nullptr);
    textwithoutnum HideNumbers(const QString &userText);
    QString restoreNumbers(const QString &airesp, QMap<QString,QString> nums);
};

#endif // PHONENUMBER_H
