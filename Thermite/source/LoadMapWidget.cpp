/*******************************************************************************
Copyright (c) 2005-2009 David Williams

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution. 	
*******************************************************************************/

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
		m_applicationGameLogic->onLoadMapClicked(pCurrentItem->text());		
	}

	void LoadMapWidget::on_m_btnClose_clicked(void)
	{
		hide();
	}
}
