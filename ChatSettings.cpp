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
	m_pAddUrl = new QPushButton("Add");
	m_pDeleteUrl = new QPushButton("Delete");
	m_pAddModel = new QPushButton("Add");
	m_pDeleteModel = new QPushButton("Delete");

	m_pUrlLabel = new QLabel("Url：");
	m_pUrlCombox = new QComboBox();
	m_pUrlCombox->setFixedWidth(200);
	QString strUrl = g_ChatDB.getSingleValue("Url")[0];
	QStringList strUrlList = strUrl.split(';');
	m_pUrlCombox->addItems(strUrlList);
	QHBoxLayout* pUrlLayout = new QHBoxLayout();
	pUrlLayout->addWidget(m_pUrlLabel);
	pUrlLayout->addWidget(m_pUrlCombox);
	pUrlLayout->addWidget(m_pAddUrl);
	pUrlLayout->addWidget(m_pDeleteUrl);
	m_pModelLabel = new QLabel("Model：");
	m_pModelCombox = new QComboBox();
	m_pModelCombox->setFixedWidth(200);
	QString strModel = g_ChatDB.getSingleValue("model")[0];
	m_pModelCombox->addItems(strModel.split(';'));
	QHBoxLayout* pModelLayout = new QHBoxLayout();
	pModelLayout->addWidget(m_pModelLabel);
	pModelLayout->addWidget(m_pModelCombox);
	pModelLayout->addWidget(m_pAddModel);
	pModelLayout->addWidget(m_pDeleteModel);
	m_pApiKeyLabel = new QLabel("ApiKey：");
	m_pApiKeyCombox = new QComboBox();
	m_pApiKeyCombox->setFixedWidth(385);
	QString strKey = g_ChatDB.getSingleValue("ApiKey")[0];
	m_pApiKeyCombox->addItems(strKey.split(';'));
	QHBoxLayout* pKeyLayout = new QHBoxLayout();
	pKeyLayout->addWidget(m_pApiKeyLabel);
	pKeyLayout->addWidget(m_pApiKeyCombox);
	setFixedWidth(500);

	//添加/删除Url
	connect(m_pAddUrl, &QPushButton::clicked, [=]() {
		QInputDialog inputDlg;
		inputDlg.setLabelText(tr("填写URL，形如:http://127.0.0.1:8080"));
		int ret = inputDlg.exec();
		if (ret == QDialog::Accepted)
		{
			QString strValue = inputDlg.textValue();
			m_pUrlCombox->insertItem(0, strValue);
			m_pUrlCombox->setCurrentIndex(0);
		}
		});
	connect(m_pDeleteUrl, &QPushButton::clicked, [=]() {
			m_pUrlCombox->removeItem(m_pUrlCombox->currentIndex());
			m_pUrlCombox->setCurrentIndex(0);
		});
	//添加/删除模型和密钥
	connect(m_pAddModel, &QPushButton::clicked, [=]() {
		QInputDialog inputDlg;
		inputDlg.setLabelText(tr("填写字符串，模型和密钥用分号分开，形如：模型名;密钥"));
		int ret = inputDlg.exec();
		if (ret == QDialog::Accepted)
		{
			QString strValue = inputDlg.textValue();
			QString strModelName = strValue.mid(0, strValue.indexOf(";"));
			QString strKeyValue = strValue.mid(strValue.indexOf(";") + 1, -1);
			if (strModelName.isEmpty() || strKeyValue.isEmpty())
			{
				QMessageBox::warning(nullptr,"警告","模型或密钥不能为空");
				return;
			}
			m_pModelCombox->insertItem(0, strModelName);
			m_pModelCombox->setCurrentIndex(0);
			m_pApiKeyCombox->insertItem(0, strKeyValue);
			m_pApiKeyCombox->setCurrentIndex(0);
		}
		});
	connect(m_pDeleteModel, &QPushButton::clicked, [=]() {
		int iCurrentIndex = m_pModelCombox->currentIndex();
		m_pModelCombox->removeItem(iCurrentIndex);
		m_pModelCombox->setCurrentIndex(0);
		m_pApiKeyCombox->removeItem(iCurrentIndex);
		m_pApiKeyCombox->setCurrentIndex(0);
		});
	//模型密钥联动
	connect(m_pModelCombox, &QComboBox::currentIndexChanged, [=](int index) {
		m_pApiKeyCombox->setCurrentIndex(index);
		});
	connect(m_pApiKeyCombox, &QComboBox::currentIndexChanged, [=](int index) {
		m_pModelCombox->setCurrentIndex(index);
		});
	
	m_pPluginList = new QListView();
	connect(m_pPluginList, &QListView::doubleClicked, [](const QModelIndex& index) {
		QString strPluginName = index.data().toString();
		QInputDialog inputDlg;
		QString strParamValue = g_ChatDB.getPluginParam(strPluginName);
		QString strNotes = g_ChatDB.getPluginNotes(strPluginName);
		inputDlg.setLabelText(tr("请按格式修改字符串，最后一个参数只能是out"));
		inputDlg.setTextValue(strParamValue + ";" + strNotes);
		int ret = inputDlg.exec();
		if (ret == QDialog::Accepted)
		{
			QString strValue = inputDlg.textValue();
			QString strParmValue = strValue.mid(0, strValue.lastIndexOf(";"));
			QString strNotes = strValue.mid(strValue.lastIndexOf(";") + 1, -1);
			if(strNotes.compare("in") == 0)
				g_ChatDB.updatePlugin(strPluginName, strParmValue, true);
			else if (strNotes.compare("out") == 0)
				g_ChatDB.updatePlugin(strPluginName, strParmValue, false);
		}

		});
	
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
	connect(m_pInPluginList, &QListView::doubleClicked, [](const QModelIndex& index) {
		QString strPluginName = index.data().toString();
		QInputDialog inputDlg;
		QString strParamValue = g_ChatDB.getPluginParam(strPluginName);
		QString strNotes = g_ChatDB.getPluginNotes(strPluginName);
		inputDlg.setLabelText(tr("请按格式修改字符串，最后一个参数只能是in"));
		inputDlg.setTextValue(strParamValue + ";" + strNotes);
		int ret = inputDlg.exec();
		if (ret == QDialog::Accepted)
		{
			QString strValue = inputDlg.textValue();
			QString strParmValue = strValue.mid(0, strValue.lastIndexOf(";"));
			QString strNotes = strValue.mid(strValue.lastIndexOf(";") + 1, -1);
			if (strNotes.compare("in") == 0)
				g_ChatDB.updatePlugin(strPluginName, strParmValue, true);
			else if (strNotes.compare("out") == 0)
				g_ChatDB.updatePlugin(strPluginName, strParmValue, false);
		}

		});
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
		QString strAllUrl = m_pUrlCombox->currentText();
		int iUrlCount = m_pUrlCombox->count();
		for (int i = 0; i < iUrlCount; i++)
		{
			if (i == m_pUrlCombox->currentIndex())
				continue;
			strAllUrl.append(';');
			strAllUrl.append(m_pUrlCombox->itemText(i));
		}
		g_ChatDB.setSingleValue("Url", strAllUrl, "");

		QString strAllModel = m_pModelCombox->currentText();
		int iModelCount = m_pModelCombox->count();
		for (int i = 0; i < iModelCount; i++)
		{
			if (i == m_pModelCombox->currentIndex())
				continue;
			strAllModel.append(';');
			strAllModel.append(m_pModelCombox->itemText(i));
		}
		g_ChatDB.setSingleValue("model", strAllModel, "");

		QString strAllKey = m_pApiKeyCombox->currentText();
		for (int i = 0; i < iModelCount; i++)
		{
			if (i == m_pApiKeyCombox->currentIndex())
				continue;
			strAllKey.append(';');
			strAllKey.append(m_pApiKeyCombox->itemText(i));
		}

		g_ChatDB.setSingleValue("ApiKey", strAllKey, "");
		emit settingsChangedSignal(m_pUrlCombox->currentText(), m_pModelCombox->currentText(), m_pApiKeyCombox->currentText());
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