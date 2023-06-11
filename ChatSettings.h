#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QStandardItemModel>
#include <QPushButton>
class QChatSettings : public QWidget
{
    Q_OBJECT
public:
    QChatSettings(QWidget *parent = nullptr);

signals:
    void settingsChangedSignal(QString strUrl,QString strModel,QString strApiKey);

private:
    QLabel* m_pUrlLabel;
    QLineEdit* m_pUrlEdit;
    QLabel* m_pModelLabel;
    QLineEdit* m_pModelEdit;
    QLabel* m_pApiKeyLabel;
    QLineEdit* m_pApiKeyEdit;
    QListView* m_pPluginList;
    QStandardItemModel* m_pModel;
    QPushButton* m_pAddPluginBtn;
    QListView* m_pInPluginList;
    QStandardItemModel* m_pInModel;
    QPushButton* m_pAddInPluginBtn;
    QPushButton* m_pSaveBtn;
};

