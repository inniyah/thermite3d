#include "ChooseMeshWidget.h"

#include <OgreEntity.h>

namespace QtOgre
{
	ChooseMeshWidget::ChooseMeshWidget(Ogre::Entity* jaiquaEntity, Ogre::Entity* robotEntity, QWidget *parent)
		:QWidget(parent, Qt::Tool)
	{
		setupUi(this);

		mJaiquaEntity = jaiquaEntity;
		mRobotEntity = robotEntity;
	}

	void ChooseMeshWidget::on_mJaiquaRadioButton_toggled(bool checked)
	{
		mRobotEntity->setVisible(!checked);
		mJaiquaEntity->setVisible(checked);
	}

	void ChooseMeshWidget::on_mRobotRadioButton_toggled(bool checked)
	{
		mRobotEntity->setVisible(checked);
		mJaiquaEntity->setVisible(!checked);
	}
}