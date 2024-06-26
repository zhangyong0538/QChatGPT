﻿#include "NetworkThread.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextCodec>
#include "ChatData.h"
#include "Tool.h"

QNetworkThread::QNetworkThread(QObject* parent):QThread(parent)
{
    connect(&m_networkManager,&QNetworkAccessManager::finished,this,&QNetworkThread::handleReply);
}

void QNetworkThread::initRequest(QString strUrl)
{
    m_request.setUrl(strUrl);

    m_request.setRawHeader("Content-Type", "application/json");
    QByteArray bArray = QString("Bearer %1").arg(m_strAppKey).toLocal8Bit();
    m_request.setRawHeader("Authorization", bArray);

}

void QNetworkThread::initJsonMsg(QString strModel)
{
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "You are a helpful assistant.";

    QJsonArray messages;
    //messages.append(systemMessage);
    m_jsonMsg["model"] = strModel;
    m_jsonMsg["messages"] = messages;
    m_jsonMsg["max_tokens"] = 1000;
    m_jsonMsg["n"] = 1;
    m_jsonMsg["stop"] = QJsonValue();  // Represents 'null' in JSON
    m_jsonMsg["temperature"] = 0.1;
    m_jsonMsg["stream"] = true;
}

void QNetworkThread::setAppKey(QString strKey,QString strUrl,QString strModel)
{
    m_strAppKey.clear();
    m_strAppKey.append(strKey);
    initRequest(strUrl);
    initJsonMsg(strModel);
}

void QNetworkThread::run()
{
    exec();
}

void QNetworkThread::onSendRequest()
{
    m_jsonMsg["messages"] = g_chatData.buildJsonMsg();

    QJsonDocument jsonDoc(m_jsonMsg);
    QByteArray postData = jsonDoc.toJson();
    m_pReply = m_networkManager.post(m_request, postData);

    connect(m_pReply, &QNetworkReply::readyRead, this, &QNetworkThread::onReadyRead);
    //updateTextBrowser();
}

void QNetworkThread::onReadyRead()
{
    if (m_pReply->error() != QNetworkReply::NoError)
    {
        QString str = m_pReply->readAll();
        QTool::Log(QString("Error:Msg=%1；Code=%2").arg(str).arg(m_pReply->error()));
        return;
    }
    QByteArray response_data = m_pReply->readAll();
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QString response_string = codec->toUnicode(response_data);
    QStringList resList = response_string.split("data: ", Qt::SkipEmptyParts);
    for (const QString& str : resList)
    {
        if (str.contains("[DONE]"))
        {
            return;
        }
        QJsonParseError perror;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8(),&perror);
        
        if (jsonResponse.isNull())
            continue;
     
        if (!jsonResponse.object()["error"].isNull() &&
            jsonResponse.object()["error"].toObject()["code"].toString().compare("context_length_exceeded") == 0)
        {
            g_chatData.deleteEarlyChats(20);//删除最早的20%
            QTool::Log(QString(tr("内容超限，已删除前边部分对话")));
            return;
        }
        else if (!jsonResponse.object()["error"].isNull())
        {
            QString errorMsg = jsonResponse.object()["error"].toObject()["message"].toString();
            g_chatData.addAnswer(errorMsg);
            emit answerSignal(errorMsg);
            QTool::Log(QString(tr("收到服务器端返回的错误信息为：%1")).arg(errorMsg));
            return;
        }
        QString responseText = "";
        //百度聊天模型
        if(m_request.url().toString().contains("baidu"))
            responseText = jsonResponse.object()["result"].toString();
        else//默认为openai
            responseText = jsonResponse.object()["choices"].toArray()[0].toObject()["delta"].toObject()["content"].toString();
        if (responseText.isNull())
            continue;
        g_chatData.addAnswer(responseText);
        emit answerSignal(responseText);
    }
}

void QNetworkThread::handleReply(QNetworkReply* reply)
{
    static int buildTimes = 0;
    //无影响读取到数据后：调试用
    QNetworkReply::NetworkError error = reply->error();
    if (error == QNetworkReply::ProtocolInvalidOperationError && buildTimes < 3)
    {
        QThread::sleep(2);
        //sendRequest();
        buildTimes++;
        //return;
    }
    else if (error != QNetworkReply::NoError && buildTimes < 3)
    {
        QThread::sleep(2);
        //sendRequest();
        buildTimes++;
        QString str = reply->readAll();
        QTool::Log(QString(tr("错误信息：%1，错误码：%2")).arg(str).arg(error));
        //return;
    }
    buildTimes = 0;
    emit finishedSignal();
}

void QNetworkThread::onSettingsChangedSignal(QString strUrl, QString strModel, QString strApiKey)
{
    setAppKey(strApiKey, strUrl, strModel);
}