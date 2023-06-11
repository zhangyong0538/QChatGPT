#pragma once
#include <QString>
#include <QList>
#include <QJsonArray>
#include <QMutex>

class QChatData
{
public:
    struct Conversion
    {
        int iConversionId;
        int iState;
        QString strRequest;
        QString strResponse;
        QString strTime;//时间格式：2023-04-30 09:19:27
    };

public:
    void addAsk(QString strAsk);
    void addAnswer(QString strAnswer);
    void updateLastState(int iState);
    int getLastState();
    void deleteEarlyChats(int delPercent);//删除较早的对话，百分比
    QJsonArray buildJsonMsg();
    int getSize();
    bool addConversion(Conversion c);
    Conversion getConversion(int i);
    Conversion getConversionByIndex(int index);
    Conversion getLastConversion();
    bool removeConversion(int id);
    bool clear();

private:
    QList<Conversion> m_chatDataList;
    QMutex m_chatDataLock;
};

extern QChatData g_chatData;
