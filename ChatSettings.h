#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QComboBox>
class QChatSettings : public QWidget
{
    Q_OBJECT
public:
    QChatSettings(QWidget *parent = nullptr);

signals:
    void settingsChangedSignal(QString strUrl,QString strModel,QString strApiKey);

private:
    QLabel* m_pUrlLabel;
    QComboBox* m_pUrlCombox;
    QLabel* m_pModelLabel;
    QComboBox* m_pModelCombox;
    QLabel* m_pApiKeyLabel;
    QComboBox* m_pApiKeyCombox;
    QListView* m_pPluginList;
    QStandardItemModel* m_pModel;
    QPushButton* m_pAddPluginBtn;
    QListView* m_pInPluginList;
    QStandardItemModel* m_pInModel;
    QPushButton* m_pAddInPluginBtn;
    QPushButton* m_pSaveBtn;

    QPushButton* m_pAddUrl;
    QPushButton* m_pDeleteUrl;
    QPushButton* m_pAddModel;
    QPushButton* m_pDeleteModel;
};

