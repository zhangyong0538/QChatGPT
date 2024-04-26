#include "ChatBrowser.h"
#include "ChatDatabase.h"
#include <QUrlQuery>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFile>

QChatBrowser::QChatBrowser(QWidget* parent) :QTextBrowser(parent)
{
	m_pVScrollBar = verticalScrollBar();
	m_pVScrollBar->setValue(m_pVScrollBar->minimum());
	m_bShowMaxFlag = true;
    m_bBuilding = false;

    connect(this, &QTextBrowser::anchorClicked, this, &QChatBrowser::onAnchorClicked);

    g_ChatDB.setDatabaseDriver("QSQLITE");
    g_ChatDB.setDatabaseName("./QChatGPT.db");
    if (!QFile::exists("./QChatGPT.db"))
    {
        g_ChatDB.createChatHistoryTable();
        g_ChatDB.createSettingsTable();
        g_ChatDB.initSettingsTable();
        g_ChatDB.createPluginTable();
    }
    QStringList plugList = g_ChatDB.getAllOutPlugins();
    foreach(auto strName, plugList)
    {
        QString param = g_ChatDB.getPluginParam(strName);
        QStringList tList = param.split(';');
        m_strPlugins.insert(strName, tList);
    }
    
}

void QChatBrowser::mousePressEvent(QMouseEvent* e)
{
    if(!m_bBuilding)
        QTextBrowser::mousePressEvent(e);
}
void QChatBrowser::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		m_bShowMaxFlag = false;
	}
	else if (e->button() == Qt::RightButton)
	{
		m_bShowMaxFlag = true;
	}
    if (!m_bBuilding)
        QTextBrowser::mouseReleaseEvent(e);
}

void QChatBrowser::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (!m_bBuilding)
        QTextBrowser::mouseDoubleClickEvent(e);
}

void QChatBrowser::addChatAsk(int id, QString strRole, QString strAsk, QString strTime, bool bSave)
{
    if (bSave)
        m_historyChatData.addAsk(strAsk);
    QString strContent;
    strContent.append("%1[%2]");
    strContent = strContent.arg(strAsk).arg(strTime);
    QTextCursor cursor(textCursor());
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat charFormat(cursor.charFormat());

    QFont fnt = charFormat.font();
    fnt.setBold(true);
    fnt.setPointSize(15);
    charFormat.setFont(fnt);
    charFormat.setForeground(QBrush(Qt::darkGreen));
    cursor.setCharFormat(charFormat);
    if(toPlainText().length() > 0)
        cursor.insertText("\n\nUser:");
    else
        cursor.insertText("User:");

    fnt = charFormat.font();
    fnt.setBold(false);
    fnt.setPointSize(12);
    charFormat.setFont(fnt);
    charFormat.setForeground(QBrush(Qt::darkGreen));
    cursor.setCharFormat(charFormat);
    cursor.insertText(strContent.append("\t"));

    
    QString delLink = tr("<a href=\"%1\">删除</a>");
    //QString saveLink = "<a href=\"%2\">保存</a><br>";
    QUrl delUrl;
    delUrl.setScheme("del");
    delUrl.setPath("command");
    delUrl.setQuery(QString("id=%1").arg(id));
    /*QUrl saveUrl;
    saveUrl.setScheme("save");
    saveUrl.setPath("command");
    saveUrl.setQuery(QString("id=%1").arg(id));*/

    delLink = delLink.arg(delUrl.toString());
    //saveLink = saveLink.arg(saveUrl.toString());
    cursor.insertHtml(delLink);
    cursor.setCharFormat(charFormat);
    cursor.insertText("\t");
    //cursor.insertHtml(saveLink);
    for (QMap<QString, QStringList>::iterator p = m_strPlugins.begin(); p != m_strPlugins.end(); p++)
    {
        QUrl pluginUrl;
        pluginUrl.setScheme(p.key());
        pluginUrl.setPath("command");
        pluginUrl.setQuery(QString("id=%1").arg(id));
        QString pluginLink = "<a href=\"%1\">%2</a>";
        pluginLink = pluginLink.arg(pluginUrl.toString()).arg(p.key());
        cursor.insertHtml(pluginLink);
        cursor.setCharFormat(charFormat);
        cursor.insertText("\t");
    }

    setOpenLinks(false);
    
    cursor.insertText("\n\n");

    onScrollMaxSignal();
}

void QChatBrowser::addChatAnswer(QString strRole, QString strAnswer, bool bSave)
{
    if (bSave)
    {
        m_historyChatData.addAnswer(strAnswer);
        m_historyChatData.updateLastState(2);
    }
    QString strContent;
    strContent.append("%1");
    strContent = strContent.arg(strAnswer);
    QTextCursor cursor(textCursor());
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat charFormat(cursor.charFormat());
    QFont fnt = charFormat.font();
    fnt.setBold(true);
    fnt.setPointSize(15);
    charFormat.setFont(fnt);
    charFormat.setForeground(QBrush(Qt::gray));
    cursor.setCharFormat(charFormat);
    cursor.insertText("Assistant: ");

    fnt = charFormat.font();
    fnt.setBold(false);
    fnt.setPointSize(12);
    charFormat.setFont(fnt);
    charFormat.setForeground(QBrush(Qt::gray));
    cursor.setCharFormat(charFormat);
    cursor.insertText(strContent);

    onScrollMaxSignal();
}

void QChatBrowser::appendChatAnswer(QString strAnswer, bool bSave)
{
    if (bSave)
        m_historyChatData.addAnswer(strAnswer);
    QString strContent;
    strContent.append("%1");
    strContent = strContent.arg(strAnswer);
    QTextCursor cursor(textCursor());
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat charFormat(cursor.charFormat());
    QFont fnt = charFormat.font();
    fnt.setBold(false);
    fnt.setPointSize(12);
    charFormat.setFont(fnt);
    charFormat.setForeground(QBrush(Qt::gray));
    cursor.setCharFormat(charFormat);
    //moveCursor(QTextCursor::End);
    cursor.insertText(strContent);

    onScrollMaxSignal();
}

void QChatBrowser::updateHistoryChatData(QString strTopic)
{
    if (strTopic.compare("") == 0)
    {
        m_historyChatData.clear();
        g_chatData.clear();
        updateChatBrowser();
        return;
    }
    m_historyChatData.clear();
    g_chatData.clear();
    QList<QChatData::Conversion> cList = g_ChatDB.getConversionList(strTopic);
    foreach(auto c , cList)
    {
        m_historyChatData.addConversion(c);
        g_chatData.addConversion(c);
    }
    updateChatBrowser();
}

void QChatBrowser::updateChatBrowser()
{
    clear();
    int size = m_historyChatData.getSize();
    for (int i = 0; i < size; i++)
    {
        QChatData::Conversion conversion = m_historyChatData.getConversionByIndex(i);
        if (conversion.iState > 0)
            addChatAsk(conversion.iConversionId, "", conversion.strRequest, conversion.strTime, false);
        if (conversion.iState > 1)
            addChatAnswer("", conversion.strResponse, false);
    }
}

void QChatBrowser::onScrollMaxSignal()
{
	if(m_bShowMaxFlag)
		m_pVScrollBar->setValue(m_pVScrollBar->maximum());
}

void QChatBrowser::onStarted()
{
    m_bBuilding = true;
    m_bShowMaxFlag = true;
    setContextMenuPolicy(Qt::NoContextMenu);
}

void QChatBrowser::onFinished()
{
    m_historyChatData.updateLastState(3);
    m_bBuilding = false;
    setContextMenuPolicy(Qt::DefaultContextMenu);
}

#include <QProcess>
void QChatBrowser::onAnchorClicked(const QUrl& link)
{
    if (link.scheme().compare("del") == 0)
    {
        int id = QUrlQuery(link.query()).queryItemValue("id").toInt();
        g_chatData.removeConversion(id);
        m_historyChatData.removeConversion(id);
        int pos = verticalScrollBar()->value();
        updateChatBrowser();
        verticalScrollBar()->setValue(pos);
        emit deleteConversionSignal(id);
    }
    else for (QMap<QString, QStringList>::iterator p = m_strPlugins.begin(); p != m_strPlugins.end(); p++)
    {
        if (link.scheme().compare(p.key(),Qt::CaseInsensitive) == 0)
        {
            int id = QUrlQuery(link.query()).queryItemValue("id").toInt();
            QChatData::Conversion c = m_historyChatData.getConversion(id);
            QProcess* exe = new QProcess();
            QString setExeName = QString("./plugins/%1").arg(p.key());
            exe->setWorkingDirectory("./plugins/");
            exe->setProgram(setExeName);
            QStringList strList = p.value();
            strList.append(c.strRequest);
            strList.append(c.strResponse);
            strList.append(c.strTime);
            exe->setArguments(strList);
            exe->start();
        }
    }
}