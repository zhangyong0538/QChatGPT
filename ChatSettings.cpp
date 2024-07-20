#include "ChatSettings.h"
#include "ChatDatabase.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

QChatSettings::QChatSettings(QWidget* parent)
{
	m_pUrlLabel = new QLabel("Url：");
	m_pUrlEdit = new QLineEdit();
	QString strUrl = g_ChatDB.getSingleValue("Url")[0];
	m_pUrlEdit->setText(strUrl);
	QHBoxLayout* pUrlLayout = new QHBoxLayout();
	pUrlLayout->addWidget(m_pUrlLabel);
	pUrlLayout->addWidget(m_pUrlEdit);
	m_pModelLabel = new QLabel("Model：");
	m_pModelEdit = new QLineEdit();
	QString strModel = g_ChatDB.getSingleValue("model")[0];
	m_pModelEdit->setText(strModel);
	QHBoxLayout* pModelLayout = new QHBoxLayout();
	pModelLayout->addWidget(m_pModelLabel);
	pModelLayout->addWidget(m_pModelEdit);
	m_pApiKeyLabel = new QLabel("ApiKey：");
	m_pApiKeyEdit = new QLineEdit();
	QString strKey = g_ChatDB.getSingleValue("ApiKey")[0];
	m_pApiKeyEdit->setText(strKey);
	QHBoxLayout* pKeyLayout = new QHBoxLayout();
	pKeyLayout->addWidget(m_pApiKeyLabel);
	pKeyLayout->addWidget(m_pApiKeyEdit);
	setFixedWidth(500);
	
	m_pPluginList = new QListView();
	//QHeaderView* header = new QHeaderView(Qt::Horizontal, m_pPluginList);
	//m_pPluginList-
	m_pModel = new QStandardItemModel();
	m_pPluginList->setModel(m_pModel);
	QStringList strList = g_ChatDB.getAllOutPlugins();
	foreach(auto str,strList)
	m_pModel->insertRow(m_pModel->rowCount(),new QStandardItem(str));

	m_pAddPluginBtn = new QPushButton(tr("添加输出插件"));
	connect(m_pAddPluginBtn, &QPushButton::clicked, [=]() {
		QInputDialog inputDlg;
		inputDlg.setLabelText(tr("填写字符串，其中最后一个参数表示是否自动执行，1-是，0-否，形如：插件名;参数1;参数2;参数3...;1"));
		int ret = inputDlg.exec();
		if (ret == QDialog::Accepted)
		{
			QString strValue = inputDlg.textValue();
			QString strName = strValue.mid(0, strValue.indexOf(";"));
			QString strPValue = strValue.mid(strValue.indexOf(";") + 1, -1);
			g_ChatDB.addPlugin(strName, strPValue,false);
			m_pModel->insertRow(m_pModel->rowCount(), new QStandardItem(strName));
		}
		});
	m_pPluginList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pPluginList, &QListView::customContextMenuRequested, [=]() {
		QMenu menu;
		QAction delAction(tr("删除"), m_pPluginList);
		connect(&delAction, &QAction::triggered, [=]() {
			g_ChatDB.deletePlugin(m_pModel->item(m_pPluginList->currentIndex().row())->text());
			m_pModel->removeRow(m_pPluginList->currentIndex().row());
			});
		menu.addAction(&delAction);
		int x = m_pPluginList->mapToGlobal(QPoint(0, 0)).x();
		int y = m_pPluginList->mapToGlobal(QPoint(0, 0)).y();
		menu.exec(QPoint(x, y));
		});


	//输入插件操作
	m_pInPluginList = new QListView();
	m_pInModel = new QStandardItemModel();
	m_pInPluginList->setModel(m_pInModel);
	QStringList strInList = g_ChatDB.getAllInPlugins();
	foreach(auto str, strInList)
		m_pInModel->insertRow(m_pInModel->rowCount(), new QStandardItem(str));

	m_pAddInPluginBtn = new QPushButton(tr("添加输入插件"));
	connect(m_pAddInPluginBtn, &QPushButton::clicked, [=]() {
		QInputDialog dlg;
		dlg.setLabelText(tr("填写字符串，形如：插件名;参数1;参数2;参数3..."));
		int ret = dlg.exec();
		if (ret == QDialog::Accepted)
		{
			QString strValue = dlg.textValue();
			QString strName = strValue.mid(0, strValue.indexOf(";"));
			QString strPValue = strValue.mid(strValue.indexOf(";") + 1, -1);
			g_ChatDB.addPlugin(strName, strPValue, true);
			m_pInModel->insertRow(m_pInModel->rowCount(), new QStandardItem(strName));
		}
		});
	m_pInPluginList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pInPluginList, &QListView::customContextMenuRequested, [=]() {
		QMenu menu(m_pPluginList);
		QAction* delAction = new QAction(tr("删除"), m_pInPluginList);
		connect(delAction, &QAction::triggered, [=]() {
			g_ChatDB.deletePlugin(m_pInModel->item(m_pInPluginList->currentIndex().row())->text());
			m_pInModel->removeRow(m_pInPluginList->currentIndex().row());
			});
		menu.addAction(delAction);
		int x = m_pInPluginList->mapToGlobal(QPoint(0,0)).x();
		int y = m_pInPluginList->mapToGlobal(QPoint(0, 0)).y();
		menu.exec(QPoint(x,y));
		});
	//输入插件操作_End

	m_pSaveBtn = new QPushButton(tr("保存"));
	connect(m_pSaveBtn, &QPushButton::clicked, [=]() {
		g_ChatDB.setSingleValue("Url", m_pUrlEdit->text(), "");
		g_ChatDB.setSingleValue("model", m_pModelEdit->text(), "");
		g_ChatDB.setSingleValue("ApiKey", m_pApiKeyEdit->text(), "");
		emit settingsChangedSignal(m_pUrlEdit->text(), m_pModelEdit->text(), m_pApiKeyEdit->text());
		QMessageBox::about(this,tr("保存"),tr("保存成功"));
		this->hide();
		});

	QVBoxLayout* pLayout = new QVBoxLayout();
	pLayout->addLayout(pUrlLayout);
	pLayout->addLayout(pModelLayout);
	pLayout->addLayout(pKeyLayout);
	pLayout->addWidget(new QLabel(tr("输入插件列表：")));
	pLayout->addWidget(m_pInPluginList);
	pLayout->addWidget(m_pAddInPluginBtn);
	pLayout->addWidget(new QLabel(tr("输出插件列表：")));
	pLayout->addWidget(m_pPluginList);
	pLayout->addWidget(m_pAddPluginBtn);
	pLayout->addWidget(m_pSaveBtn);
	setLayout(pLayout);
}