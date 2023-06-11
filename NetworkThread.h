#pragma once
#include <qthread.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QJsonObject>
#include <QString>

class QNetworkThread : public QThread
{
    Q_OBJECT
public:
    QNetworkThread(QObject*parent=nullptr);
    void initRequest(QString strUrl);
    void initJsonMsg(QString strModel);
    void setAppKey(QString strKey, QString strUrl, QString strModel);
    void run();

signals:
    void answerSignal(QString strAnswer);
    void finishedSignal();

public slots:
    void onSendRequest();
    void onReadyRead();
    void handleReply(QNetworkReply* reply);
    void onSettingsChangedSignal(QString strUrl, QString strModel, QString strApiKey);

private:
    QNetworkAccessManager m_networkManager;
    QNetworkRequest m_request;
    QNetworkReply* m_pReply;
    QString m_strAppKey;
    QJsonObject m_jsonMsg;
};

