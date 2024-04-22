#include "ChatGPTClient.h"
#include "ChatData.h"
#include "ChatDatabase.h"
#include <QJsonArray>
#include <QTextCodec>
#include <QScrollBar>
#include <QMessageBox>
#include <QTextBlock>
#include <QTextCursor>
#include <QDir>
#include <QFile>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMenu>
#include <QProcess>

ChatGPTClient::ChatGPTClient(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(600,400);
    QVBoxLayout* layout = new QVBoxLayout();
    
    m_pSendBtn = new QPushButton(tr("发送"));
    m_inputLineEdit = new QLineEdit(this);
    m_inputLineEdit->setFixedHeight(50);
    m_pChatBrowser = new QChatBrowser(this);
    m_pAddTopicBtn = new QPushButton(tr("添加"));
    m_pListWidget = new QListWidget();
    m_pSettingsBtn = new QPushButton(tr("设置"));
    //输入插件
    QWidget* pluginWidget = new QWidget();
    pluginWidget->setFixedHeight(100);

    QVBoxLayout* pLeftLayout = new QVBoxLayout();
    pLeftLayout->addWidget(m_pAddTopicBtn);
    pLeftLayout->addWidget(m_pListWidget);
    pLeftLayout->addWidget(pluginWidget);
    pLeftLayout->addWidget(m_pSettingsBtn);
    
    layout->addWidget(m_pChatBrowser); 
    QHBoxLayout* pHBoxLayout = new QHBoxLayout();
    pHBoxLayout->addWidget(m_inputLineEdit);
    pHBoxLayout->addWidget(m_pSendBtn);
    layout->addLayout(pHBoxLayout);

    QHBoxLayout* pAllLayout = new QHBoxLayout();
    pAllLayout->addLayout(pLeftLayout,1);
    pAllLayout->addLayout(layout,5);
    setLayout(pAllLayout);

    QFont fnt;
    fnt.setBold(1);
    fnt.setPixelSize(20);
    m_pListWidget->setFont(fnt);
    m_pListWidget->setSpacing(8);
    m_pListWidget->setEditTriggers(QListWidget::DoubleClicked);
    connect(m_pListWidget, &QListWidget::currentItemChanged, this, &ChatGPTClient::onTopicChanged);
    m_pListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pListWidget, &QListWidget::customContextMenuRequested, this, &ChatGPTClient::showContextMenu);
    connect(m_pListWidget, &QListWidget::itemChanged, this, &ChatGPTClient::onItemChanged);
    connect(m_pListWidget, &QListWidget::itemDoubleClicked, this, &ChatGPTClient::onItemDoubleClicked);


    m_networkThread.moveToThread(&m_networkThread);
    connect(m_inputLineEdit, &QLineEdit::returnPressed, this, &ChatGPTClient::sendRequest);
    connect(m_pSendBtn, &QPushButton::clicked, this, &ChatGPTClient::sendRequest);
    connect(this, &ChatGPTClient::sendRequestSignal, &m_networkThread, &QNetworkThread::onSendRequest);
    connect(&m_networkThread, &QNetworkThread::answerSignal, this, &ChatGPTClient::onAnswer);
    connect(&m_networkThread, &QNetworkThread::finishedSignal, this, &ChatGPTClient::onAnswerFinished);
    connect(this, &ChatGPTClient::sendRequestSignal, m_pChatBrowser, &QChatBrowser::onStarted);
    connect(&m_networkThread, &QNetworkThread::finishedSignal, m_pChatBrowser, &QChatBrowser::onFinished);
    connect(m_pAddTopicBtn, &QPushButton::clicked, this, &ChatGPTClient::onAddTopicBtn);
    connect(m_pSettingsBtn, &QPushButton::clicked, this, &ChatGPTClient::onSettingsBtn);
    connect(m_pChatBrowser, &QChatBrowser::deleteConversionSignal, this, &ChatGPTClient::onDeleteConversion);
    
    g_ChatDB.setDatabaseDriver("QSQLITE");
    g_ChatDB.setDatabaseName("./QChatGPT.db");
    if (!QFile::exists("./QChatGPT.db"))
    {
        g_ChatDB.createChatHistoryTable();
        g_ChatDB.createSettingsTable();
        g_ChatDB.initSettingsTable();
        g_ChatDB.createPluginTable();
    }
    QStringList url = g_ChatDB.getSingleValue("Url");
    QStringList model = g_ChatDB.getSingleValue("model");
    QStringList key = g_ChatDB.getSingleValue("ApiKey");
    m_networkThread.setAppKey(key[0], url[0], model[0]);
    m_networkThread.start();

    QStringList topics = g_ChatDB.getAllTopics();
    for(QString topic : topics)
    {
        QListWidgetItem* pItem = new QListWidgetItem(topic);
        pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
        m_pListWidget->addItem(pItem);
    }
    m_pListWidget->setCurrentItem(m_pListWidget->item(0));
    
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, [=](const QPoint& pos) {
        QMenu* menu = new QMenu(this);
        menu->addAction("Action 1");
        menu->addAction("Action 2");
        menu->exec(this->mapToGlobal(pos));
        });

    m_pSettings = new QChatSettings();
    connect(m_pSettings, &QChatSettings::settingsChangedSignal, &m_networkThread, &QNetworkThread::onSettingsChangedSignal);

    QVBoxLayout* ppluginWidgetLayout = new QVBoxLayout();
    QLabel* pPluginLabel = new QLabel(tr("输入插件"));
    ppluginWidgetLayout->addWidget(pPluginLabel);
    //QScrollBar* pPluginScrollBar = new QScrollBar(Qt::Orientation::Vertical, pluginWidget);
    //QVBoxLayout* pScrollBarLayout = new QVBoxLayout();
    QStringList plugList = g_ChatDB.getAllInPlugins();
    foreach(auto strName, plugList)
    {
        QString param = g_ChatDB.getPluginParam(strName);
        QStringList tList = param.split(';');
        m_strPlugins.insert(strName, tList);
        //创建QCheckBox
        QCheckBox* pCheckBox = new QCheckBox(strName);
        pCheckBox->setFixedHeight(50);
        m_InputPluginList.push_back(pCheckBox);
        ppluginWidgetLayout->addWidget(pCheckBox);
    }
    //pPluginScrollBar->setLayout(pScrollBarLayout);
    //ppluginWidgetLayout->addWidget(pPluginScrollBar);
    pluginWidget->setLayout(ppluginWidgetLayout);
}

ChatGPTClient::~ChatGPTClient()
{
    m_networkThread.quit();
    m_networkThread.wait();
    delete m_pChatBrowser;
}


void ChatGPTClient::mouseDoubleClickEvent(QMouseEvent* e)
{
    int a = 1;
    int b = a;
}


void ChatGPTClient::sendRequest() 
{
    if (m_pListWidget->count() == 0)
        onAddTopicBtn();
    QString strInput = m_inputLineEdit->text();
    m_inputLineEdit->clear();
    m_pAddTopicBtn->setEnabled(false);
    m_pListWidget->setEnabled(false);
    m_pSendBtn->setEnabled(false);
    m_pSendBtn->setText(tr("生成中..."));
    QCoreApplication::processEvents();
    foreach(auto p, m_InputPluginList)
    {
        if (p->isChecked())
        {
            QProcess* exe = new QProcess();
            QString setExeName = QString("./plugins/%1").arg(p->text());
            exe->setWorkingDirectory("./plugins/");
            exe->setProgram(setExeName);
            QString str0 = g_ChatDB.getPluginParam(p->text());
            QStringList strList = str0.split(';');
            strList.append(strInput);
            exe->setArguments(strList);
            exe->start();
            exe->waitForFinished(100000);
            strInput = exe->readAllStandardOutput();
            if (strInput.isEmpty())//备用，收不到时读取备份文件
            {
                QFile rFile("./plugins/Out.bak");
                rFile.open(QIODevice::ReadOnly);
                strInput = rFile.readAll();
                rFile.close();
                rFile.remove();
            }
            break;
        }
    }
    if (strInput.isEmpty())
    {
        m_pAddTopicBtn->setEnabled(true);
        m_pListWidget->setEnabled(true);
        m_pSendBtn->setEnabled(true);
        m_pSendBtn->setText(tr("发送"));
        return;
    }
    if (m_strCurrentTopic.compare("NEW") == 0)
    {
        m_strCurrentTopic = g_ChatDB.buildTopicName(strInput);
        m_pListWidget->item(0)->setText(m_strCurrentTopic);
    }
    g_chatData.addAsk(strInput);
    g_chatData.updateLastState(1);
    int chatSize = g_chatData.getSize();
    QChatData::Conversion lastChat = g_chatData.getConversionByIndex(chatSize - 1);
    m_pChatBrowser->addChatAsk(lastChat.iConversionId, "User", strInput, lastChat.strTime);
    m_inputLineEdit->clear();
    m_pChatBrowser->verticalScrollBar()->setValue(m_pChatBrowser->verticalScrollBar()->maximum());
    emit sendRequestSignal();
}

void ChatGPTClient::onAnswer(QString strAnswer)
{
    if (g_chatData.getLastState() == 1)
    {
        m_pChatBrowser->addChatAnswer("", strAnswer);
        g_chatData.updateLastState(2);
    }
    else
        m_pChatBrowser->appendChatAnswer(strAnswer);
}

void ChatGPTClient::onAnswerFinished()
{
    g_chatData.updateLastState(3);
    m_pAddTopicBtn->setEnabled(true);
    m_pListWidget->setEnabled(true);
    m_pSendBtn->setText(tr("发送"));
    m_pSendBtn->setEnabled(true);
    g_ChatDB.addNewConversion(m_pListWidget->currentItem()->text());
}

void ChatGPTClient::onAddTopicBtn()
{
    if (m_pListWidget->count() > 0 && m_pListWidget->item(0)->text().compare("NEW") == 0)
    {
        m_pListWidget->setCurrentItem(m_pListWidget->item(0));
        m_strCurrentTopic = "NEW";
        m_pChatBrowser->updateHistoryChatData("");
        return;
    }
    QListWidgetItem* pItem = new QListWidgetItem("NEW");
    pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
    m_pListWidget->insertItem(0, pItem);
    m_strCurrentTopic = "NEW";
    m_pChatBrowser->updateHistoryChatData("");
    m_pListWidget->setCurrentItem(m_pListWidget->item(0));
}

void ChatGPTClient::onSettingsBtn()
{
    m_pSettings->show();
}

void ChatGPTClient::onTopicChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    if (current == nullptr)
        return;
    m_strCurrentTopic = current->text();
    m_pChatBrowser->updateHistoryChatData(current->text());
}

void ChatGPTClient::showContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);
    QAction* delAction = new QAction(tr("删除"), this);
    connect(delAction, &QAction::triggered, [=]() {
        QListWidgetItem *pItem = m_pListWidget->takeItem(m_pListWidget->currentRow());
        g_ChatDB.deleteTopic(pItem->text());
        delete pItem;
        });
    menu->addAction(delAction);
    menu->popup(m_pListWidget->viewport()->mapToGlobal(pos));
}

void ChatGPTClient::onItemChanged(QListWidgetItem* item)
{
    QString strNewTopic = item->text();
    g_ChatDB.renameTopicName(m_strPreTopicName,item->text());
}

void ChatGPTClient::onItemDoubleClicked(QListWidgetItem* item)
{
    m_strPreTopicName = item->text();
    int b = 2;
}

void ChatGPTClient::onDeleteConversion(int iConversionId)
{
    g_ChatDB.deleteConversion(m_pListWidget->currentItem()->text(),iConversionId);
    if (g_ChatDB.getConversionList(m_pListWidget->currentItem()->text()).size() <= 0)
    {
        QListWidgetItem* pItem = m_pListWidget->takeItem(m_pListWidget->currentRow());
        g_ChatDB.deleteTopic(pItem->text());
        delete pItem;
    }
}