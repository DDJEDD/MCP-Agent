#include "requests.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

namespace {
QNetworkAccessManager *sharedManager()
{
    static QNetworkAccessManager *manager = new QNetworkAccessManager();
    return manager;
}
}

void Requests::apiCall(QObject *parent, const QString &host, const QString &path,
                       const QJsonObject &body,
                       const QMap<QString, QString> &headers,
                       std::function<void(const QJsonObject &)> callback){
    QUrl url(QString("https://%1%2").arg(host, path));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(35000);
    for (auto it = headers.begin(); it != headers.end(); ++it)
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());

    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    QNetworkReply *reply = sharedManager()->post(request, payload);

    if (parent) {
        QObject::connect(parent, &QObject::destroyed, reply, [reply]() {
            reply->abort();
            reply->deleteLater();
        });
    }

    QObject::connect(reply, &QNetworkReply::finished, reply, [reply, callback]() {
        const QByteArray data = reply->readAll();
        const bool hadError = reply->error() != QNetworkReply::NoError;
        if (hadError)
            qWarning() << "Requests: network error:" << reply->errorString();

        QJsonParseError parseError;
        const auto doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            if (!hadError)
                qWarning() << "Requests: JSON parse error:" << parseError.errorString()
                           << "\nBody (first 500 bytes):" << data.left(500);
            callback(QJsonObject());
        } else {
            callback(doc.object());
        }
        reply->deleteLater();
    });
}

void Requests::downloadFileCall(QObject *parent,
                                const QString &host,
                                const QString &path,
                                std::function<void(const QByteArray &)> callback) {
    QUrl url(QString("https://%1%2").arg(host, path));
    QNetworkRequest request(url);
    request.setTransferTimeout(35000);

    QNetworkReply *reply = sharedManager()->get(request);

    if (parent) {
        QObject::connect(parent, &QObject::destroyed, reply, [reply]() {
            reply->abort();
            reply->deleteLater();
        });
    }

    QObject::connect(reply, &QNetworkReply::finished, reply, [reply, callback]() {
        if (reply->error() != QNetworkReply::NoError)
            qWarning() << "Requests: download error:" << reply->errorString();
        callback(reply->readAll());
        reply->deleteLater();
    });
}
