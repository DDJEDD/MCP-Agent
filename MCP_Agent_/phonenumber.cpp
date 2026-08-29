#include "phonenumber.h"
#include <QRegularExpression>
#include <QHash>
#include <QObject>
phonenumber::phonenumber(QObject *parent) : QObject(parent) {

}

textwithoutnum phonenumber::HideNumbers(const QString &userText){
    textwithoutnum res;
    QRegularExpression phoneRegex("(?:\\+?\\d[\\d\\s\\(\\)-]{6,}\\d)");
    QRegularExpressionMatchIterator it = phoneRegex.globalMatch(userText);
    int counter = 1;
    res.usertext = userText;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString realPhone = match.captured(0);

        if (!res.numbers.values().contains(realPhone)) {
            QString tag = QString("[NUM_%1]").arg(counter++);
            res.numbers.insert(tag, realPhone);
        }
    }

    for (auto e = res.numbers.constBegin(); e != res.numbers.constEnd(); ++e) {
        res.usertext.replace(e.value(), e.key());
    }

    return res;
}

QString phonenumber::restoreNumbers(const QString &airesp, QMap<QString,QString> nums){
    QString restoredtext = airesp;
    for (auto e = nums.constBegin(); e != nums.constEnd(); ++e) {
        restoredtext.replace(e.key(), e.value());
    }

    return restoredtext;
}
