#pragma once
#include <QApplication>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QTextBrowser>
#include <QList>
#include <QString>
#include <QListWidget>
#include <QPoint>
#include <QMap>
#include <QCheckBox>
#include <QScrollArea>
#include "ChatBrowser.h"
#include "NetworkThread.h"
#include "ChatSettings.h"

class ChatGPTClient : public QWidget {
    Q_OBJECT
public:
    ChatGPTClient(QWidget* parent = nullptr);
    ~ChatGPTClient();
    void mouseDoubleClickEvent(QMouseEvent* e);

signals:
    void sendRequestSignal();

private slots:
    void onInputPreProcess();
    void sendRequest();
    void onAnswer(QString strAnswer);
    void onAnswerFinished();
    void onAddTopicBtn();
    void onSettingsBtn();
    void onTopicChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void showContextMenu(const QPoint& pos);
    void onItemChanged(QListWidgetItem* item);
    void onItemDoubleClicked(QListWidgetItem* item);
    void onDeleteConversion(int iConversionId);

private:
    QLineEdit* m_inputLineEdit;
    QPushButton* m_pSendBtn;
    QChatBrowser* m_pChatBrowser;
    QNetworkThread m_networkThread;
    QPushButton* m_pAddTopicBtn;
    QListWidget* m_pListWidget;
    QString m_strCurrentTopic;
    QString m_strPreTopicName;
    QChatSettings* m_pSettings;
    QPushButton* m_pSettingsBtn;
    QList<QCheckBox*> m_InputPluginList;
    QList<QPushButton*> m_InputPluginListBtn;
    QMap<QString, QStringList> m_strPlugins;

    QScrollArea* m_pInputPluginScrollArea;
};

