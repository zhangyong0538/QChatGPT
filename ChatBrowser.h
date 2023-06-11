#pragma once
#include <qtextbrowser.h>
#include <QTextBrowser>
#include <QMouseEvent>
#include <QScrollBar>
#include <QUrl>
#include <QString>
#include <QMap>
#include <QStringList>
#include "ChatData.h"

class QChatBrowser :public QTextBrowser
{
	Q_OBJECT
public:
	 QChatBrowser(QWidget*parent=nullptr);
	 virtual void mousePressEvent(QMouseEvent* e) override;
	 virtual void mouseReleaseEvent(QMouseEvent* e) override;
	 virtual void mouseDoubleClickEvent(QMouseEvent* e) override;
	 void addChatAsk(int id, QString strRole, QString strAsk, QString strTime, bool bSave = true);
	 void addChatAnswer(QString strRole, QString strAnswer, bool bSave = true);
	 void appendChatAnswer(QString strAnswer, bool bSave = true);
	 void updateHistoryChatData(QString strTopic);
	 void updateChatBrowser();

signals:
	void deleteConversionSignal(int iConversionId);

public slots:
	void onScrollMaxSignal();
	void onStarted();
	void onFinished();
	void onAnchorClicked(const QUrl& link);

private:
	QScrollBar* m_pVScrollBar;
	bool m_bShowMaxFlag;
	bool m_bBuilding;
	QChatData m_historyChatData;
	QMap<QString,QStringList> m_strPlugins;
};

