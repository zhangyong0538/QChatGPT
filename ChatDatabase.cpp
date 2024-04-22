#include "ChatDatabase.h"
#include "ChatData.h"
#include "Tool.h"

QChatDatabase g_ChatDB;

QChatDatabase::QChatDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

void QChatDatabase::setDatabaseDriver(QString strDriver)
{
    m_db = QSqlDatabase::addDatabase(strDriver);
}

void QChatDatabase::setDatabaseName(QString str)
{
	m_strDBName = str;
    m_db.setDatabaseName(str);
}
#include <QSqlError>
void QChatDatabase::createChatHistoryTable()
{
    if (m_db.open())
    {
        m_db.exec("CREATE TABLE chat_history (id INTEGER PRIMARY KEY, TopicName VARCHAR(50), iConversionId INTEGER, strRequest TEXT, strResponse TEXT, strTime VARCHAR(50), notes TEXT)");
        m_db.close();
    }
    QString str = m_db.lastError().text();
    int a = 2;
}

QStringList QChatDatabase::getAllTopics()
{
    QStringList tops;
    if (m_db.open())
    {
        QSqlQuery q = m_db.exec("select DISTINCT TopicName from chat_history ORDER BY id DESC");
        while (q.next())
            tops.push_back(q.value("TopicName").toString());
        m_db.close();
    }
    return tops;
}

void QChatDatabase::renameTopicName(QString strOldName, QString strNewName)
{
    strOldName = strOldName.replace("'", "''");
    strNewName = strNewName.replace("'", "''");
    if (m_db.open())
    {
        m_db.exec(QString("update chat_history set TopicName = '%1' where TopicName='%2'").arg(strNewName).arg(strOldName));
        m_db.close();
    }
}

void QChatDatabase::deleteTopic(QString strTopic)
{
    strTopic = strTopic.replace("'", "''");
    if (m_db.open())
    {
        m_db.exec(QString("delete from chat_history where TopicName='%1'").arg(strTopic));
        m_db.close();
    }
}

QString QChatDatabase::buildTopicName(QString strTopic)
{
    if (m_db.open())
    {
        QString str = "select * from chat_history where TopicName = '%1'";
        strTopic = strTopic.replace("'", "''");
        str = str.arg(strTopic);
        QSqlQuery q = m_db.exec(str);
        int index = 1;
        QString strPreTopic = strTopic;
        while (q.next())//新对话检查是否有重名
        {
            str = "select * from chat_history where TopicName = '%1'";
            strTopic = strPreTopic + QString("_%1").arg(index++);
            str = str.arg(strTopic);
            q = m_db.exec(str);
        }
        m_db.close();
    }
    return strTopic;
}

void QChatDatabase::addNewConversion(QString strTopic)
{
    if (m_db.open())
    {
        QChatData::Conversion c = g_chatData.getLastConversion();
        QString strInsert = "insert into chat_history (TopicName, iConversionId, strRequest, strResponse, strTime, notes) values\
                            ('%1',%2,'%3','%4','%5','%6')";
        QString strReq = c.strRequest.replace("'","''");
        QString strRes = c.strResponse.replace("'", "''");
        strTopic = strTopic.replace("'", "''");
        strInsert = strInsert.arg(strTopic).arg(c.iConversionId).arg(strReq).arg(strRes).arg(c.strTime).arg("问答");

        m_db.exec(strInsert);
        m_db.close();
    }
}

QList<QChatData::Conversion> QChatDatabase::getConversionList(QString strTopic)
{
    QList<QChatData::Conversion> cList;
    if (m_db.open())
    {
        QString str = "select * from chat_history where TopicName = '%1'";
        strTopic = strTopic.replace("'", "''");
        str = str.arg(strTopic);
        QSqlQuery q = m_db.exec(str);
        while (q.next())//新对话检查是否有重名
        {
            QChatData::Conversion c;
            c.iConversionId = q.value("iConversionId").toInt();
            c.iState = 3;
            c.strRequest = q.value("strRequest").toString();
            c.strResponse = q.value("strResponse").toString();
            c.strTime = q.value("strTime").toString();
            cList.push_back(c);
        }
        m_db.close();
    }
    return cList;
}

void QChatDatabase::deleteConversion(QString strTopic, int iConversionId)
{
    strTopic = strTopic.replace("'", "''");
    if (m_db.open())
    {
        m_db.exec(QString("delete from chat_history where TopicName='%1' and iConversionId=%2").arg(strTopic).arg(iConversionId));
        m_db.close();
    }
}

void QChatDatabase::createSettingsTable()
{
    if (m_db.open())
    {
        m_db.exec("CREATE TABLE settings (id INTEGER PRIMARY KEY, SettingsName VARCHAR(50), SettingsValue VARCHAR(50), notes TEXT)");
        m_db.close();
    }
}

void QChatDatabase::initSettingsTable()
{
    if (m_db.open())
    {
        m_db.exec(QString("insert into settings (SettingsName,SettingsValue,notes) values('%1','%2','%3')")
            .arg("Url").arg("https://api.openai.com/v1/chat/completions").arg("OpenAI chat URL"));
        m_db.exec(QString("insert into settings (SettingsName,SettingsValue,notes) values('%1','%2','%3')")
            .arg("model").arg("gpt-3.5-turbo").arg("OpenAI most cost-effective chat model"));
        m_db.exec(QString("insert into settings (SettingsName,SettingsValue,notes) values('%1','%2','%3')")
            .arg("ApiKey").arg("test key").arg("api key"));
        QString str = m_db.lastError().text();
        m_db.exec(QString("insert into settings (SettingsName,SettingsValue,notes) values('%1','%2','%3')")
            .arg("temperature").arg("0.5").arg("What sampling temperature to use, between 0 and 2. Higher values like 0.8 will make the output more random, while lower values like 0.2 will make it more focused and deterministic."));
        m_db.close();
    }
}

QStringList QChatDatabase::getSingleValue(QString strParName)
{
    QStringList strParam;
    if (m_db.open())
    {
        QString str = "select * from settings where SettingsName = '%1'";
        str = str.arg(strParName);
        QSqlQuery q = m_db.exec(str);
        while (q.next())
        {
            strParam.push_back(q.value("SettingsValue").toString());
            strParam.push_back(q.value("notes").toString());
            break;
        }
        m_db.close();
    }
    return strParam;
}

void QChatDatabase::setSingleValue(QString strParName, QString strValue, QString strNotes)
{
    QStringList strParam;
    if (m_db.open())
    {
        QString str = "update settings set SettingsValue='%1' where SettingsName = '%2'";
        str = str.arg(strValue).arg(strParName);
        QSqlQuery q = m_db.exec(str);
        m_db.close();
    }
}

void QChatDatabase::createPluginTable()
{
    if (m_db.open())
    {
        m_db.exec("CREATE TABLE plugin (id INTEGER PRIMARY KEY, pluginName VARCHAR(50), parmValue TEXT, notes TEXT)");
        m_db.close();
    }
}

QStringList QChatDatabase::getAllPlugins()
{
    QStringList strPlugins;
    if (m_db.open())
    {
        QString str = "select * from plugin";
        QSqlQuery q = m_db.exec(str);
        while (q.next())
        {
            strPlugins.push_back(q.value("pluginName").toString());
        }
        m_db.close();
    }
    return strPlugins;
}

QStringList QChatDatabase::getAllInPlugins()
{
    QStringList strPlugins;
    if (m_db.open())
    {
        QString str = "select * from plugin where notes='in'";
        QSqlQuery q = m_db.exec(str);
        while (q.next())
        {
            strPlugins.push_back(q.value("pluginName").toString());
        }
        m_db.close();
    }
    return strPlugins;
}

QStringList QChatDatabase::getAllOutPlugins()
{
    QStringList strPlugins;
    if (m_db.open())
    {
        QString str = "select * from plugin where notes='out'";
        QSqlQuery q = m_db.exec(str);
        while (q.next())
        {
            strPlugins.push_back(q.value("pluginName").toString());
        }
        m_db.close();
    }
    return strPlugins;
}

QString QChatDatabase::getPluginParam(QString strPluginName)
{
    QString strParam;
    if (m_db.open())
    {
        QString str = "select * from plugin where pluginName = '%1'";
        str = str.arg(strPluginName);
        QSqlQuery q = m_db.exec(str);
        while (q.next())
        {
            strParam = q.value("parmValue").toString();
            break;
        }
        m_db.close();
    }
    return strParam;
}

void QChatDatabase::addPlugin(QString strPluginName, QString strParam, bool bInOrOutput)
{
    if (m_db.open())
    {
        if(bInOrOutput)
            m_db.exec(QString("insert into plugin (pluginName,parmValue,notes) values('%1','%2','%3')")
            .arg(strPluginName).arg(strParam).arg("in"));
        else
            m_db.exec(QString("insert into plugin (pluginName,parmValue,notes) values('%1','%2','%3')")
                .arg(strPluginName).arg(strParam).arg("out"));
        m_db.close();
    }
}

void QChatDatabase::deletePlugin(QString strPluginName)
{
    if (m_db.open())
    {
        m_db.exec(QString("delete from plugin where pluginName='%1'").arg(strPluginName));
        m_db.close();
    }
}