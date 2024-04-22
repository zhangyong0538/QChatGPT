#include "ChatData.h"
#include <QJsonObject>
#include <QMutexLocker>

QChatData g_chatData;

void QChatData::addAsk(QString strAsk)
{
    QMutexLocker tLocker(&m_chatDataLock);
    if (strAsk.compare("") != 0)
    {
        Conversion tConversion;
        int id = 0;
        if(m_chatDataList.size() > 0)
            id = m_chatDataList[m_chatDataList.size() - 1].iConversionId + 1;
        tConversion.iConversionId = id;
        tConversion.iState = 1;
        tConversion.strRequest = strAsk;
        tConversion.strTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        m_chatDataList.push_back(tConversion);
    }
}

void QChatData::addAnswer(QString strAnswer)
{
    QMutexLocker tLocker(&m_chatDataLock);
    int nCount = m_chatDataList.size();
    if (nCount <= 0)
        return;
    if (m_chatDataList[nCount - 1].iState >= 3)
        return;
    m_chatDataList[nCount - 1].strResponse += strAnswer;
}

void QChatData::updateLastState(int iState)
{
    QMutexLocker tLocker(&m_chatDataLock);
    int nCount = m_chatDataList.size();
    if (nCount <= 0)
        return;
    m_chatDataList[nCount - 1].iState = iState;
}

int QChatData::getLastState()
{
    QMutexLocker tLocker(&m_chatDataLock);
    int nCount = m_chatDataList.size();
    if (nCount <= 0)
        return -1;
    return m_chatDataList[nCount - 1].iState;
}

void QChatData::deleteEarlyChats(int delPercent)
{
    QMutexLocker tLocker(&m_chatDataLock);
    int msgSize = m_chatDataList.size();
    for (int i = 0; i < msgSize * delPercent*0.01; i++)
    {
        m_chatDataList.removeAt(i);
    }
}

QJsonArray QChatData::buildJsonMsg()
{
    QMutexLocker tLocker(&m_chatDataLock);
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "You are a helpful assistant.";
    QJsonArray messages;
    //messages.append(systemMessage);
    for (QList<Conversion>::iterator p = m_chatDataList.begin(); p != m_chatDataList.end(); p++)
    {
        if (p->iState > 0)
        {
            QJsonObject userMessage;
            userMessage["role"] = "user";
            userMessage["content"] = p->strRequest;
            messages.append(userMessage);
        }
        if (p->iState > 1)
        {
            QJsonObject userMessage;
            if (1/*m_request.url().toString().contains("baidu")*/)
                userMessage["role"] = "assistant";
            else
                userMessage["role"] = "user";
            userMessage["content"] = p->strResponse;
            messages.append(userMessage);
        }
    }
    return messages;
}

int QChatData::getSize()
{
    return m_chatDataList.size();
}

bool QChatData::addConversion(Conversion c)
{
    m_chatDataList.push_back(c);
    return true;
}

QChatData::Conversion QChatData::getConversion(int id)
{
    QMutexLocker tLocker(&m_chatDataLock);
    for (int i = 0; i < m_chatDataList.size(); i++)
    {
        if(m_chatDataList[i].iConversionId == id)
            return m_chatDataList[i];
    }
    return Conversion();
}

QChatData::Conversion QChatData::getConversionByIndex(int index)
{
    QMutexLocker tLocker(&m_chatDataLock);
    return m_chatDataList[index];
}

QChatData::Conversion QChatData::getLastConversion()
{
    QMutexLocker tLocker(&m_chatDataLock);
    assert(m_chatDataList.size() > 0);
    return m_chatDataList[m_chatDataList.size()-1];
}

bool QChatData::removeConversion(int id)
{
    QMutexLocker tLocker(&m_chatDataLock);
    int i = 0;
    for (; i < m_chatDataList.size(); i++)
    {
        if (m_chatDataList[i].iConversionId == id)
            break;
    }
    if (i == m_chatDataList.size())
        return false;
    m_chatDataList.remove(i);
    return false;
}

bool QChatData::clear()
{
    QMutexLocker tLocker(&m_chatDataLock);
    m_chatDataList.clear();
    return true;
}