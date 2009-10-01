#include "LoadMapWidget.h"

#include "ApplicationGameLogic.h"

#include <OgreResourceGroupManager.h>


namespace Thermite
{
	LoadMapWidget::LoadMapWidget(ApplicationGameLogic* applicationGameLogic, QWidget* parent, Qt::WindowFlags f)
		:QWidget(parent, f)
		,m_applicationGameLogic(applicationGameLogic)
	{
		setupUi(this);

		m_btnLoad->setEnabled(false);

		bool bItemsAdded = false;

		Ogre::FileInfoListPtr list = Ogre::ResourceGroupManager::getSingletonPtr()->findResourceFileInfo(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "*.map");
		for(Ogre::FileInfoList::iterator iter = list->begin(); iter != list->end(); ++iter)
		{
			new QListWidgetItem(QString::fromStdString(iter->filename), m_listMap);
			bItemsAdded = true;
		}

		if(bItemsAdded) //There's probably a better way to determine whether there is anything in the list...
		{
			m_listMap->setCurrentRow(0);
			m_btnLoad->setEnabled(true);
		}
	}

	void LoadMapWidget::on_m_listMap_currentTextChanged(const QString & currentText)
	{
		m_strCurrentText = currentText;
		m_btnLoad->setEnabled(true);
	}

	void LoadMapWidget::on_m_btnLoad_clicked(void)
	{
		hide();

		QListWidgetItem* pCurrentItem = m_listMap->currentItem();
		m_applicationGameLogic->loadMapWrapper(pCurrentItem->text());		
	}

	void LoadMapWidget::on_m_btnClose_clicked(void)
	{
		hide();
	}
}
