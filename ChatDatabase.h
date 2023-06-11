#pragma once
#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include "ChatData.h"

class QChatDatabase
{
public:
	QChatDatabase();
	void setDatabaseDriver(QString strDriver);
	void setDatabaseName(QString str);
	void createChatHistoryTable();
	QStringList getAllTopics();
	void renameTopicName(QString strOldName,QString strNewName);
	void deleteTopic(QString strTopic);
	QString buildTopicName(QString strTopic);
	void addNewConversion(QString strTopic);
	QList<QChatData::Conversion> getConversionList(QString strTopic);
	void deleteConversion(QString strTopic,int iConversionId);
	//设置相关
	void createSettingsTable();
	void initSettingsTable();
	QStringList getSingleValue(QString strParName);
	void setSingleValue(QString strParName,QString strValue,QString strNotes);
	//插件相关
	void createPluginTable();
	QStringList getAllPlugins();
	QStringList getAllInPlugins();
	QStringList getAllOutPlugins();
	QString getPluginParam(QString strPluginName);
	void addPlugin(QString strPluginName,QString strParam,bool bInOrOutput);
	void deletePlugin(QString strPluginName);

private:
	QString m_strDBName;
	QSqlDatabase m_db;
};

extern QChatDatabase g_ChatDB;