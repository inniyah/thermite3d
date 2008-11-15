#ifndef CHOOSEMESHWIDGET_H_
#define CHOOSEMESHWIDGET_H_

#include "../ui_ChooseMeshWidget.h"

#include "OgrePrerequisites.h"

namespace QtOgre
{
	class DemoGameLogic;

	class ChooseMeshWidget : public QWidget, private Ui::ChooseMeshWidget
	{
		Q_OBJECT

	public:
		ChooseMeshWidget(Ogre::Entity* jaiquaEntity, Ogre::Entity* robotEntity, QWidget *parent = 0);

	public slots:
		void on_mJaiquaRadioButton_toggled(bool checked);
		void on_mRobotRadioButton_toggled(bool checked);

	private:
		Ogre::Entity* mJaiquaEntity;
		Ogre::Entity* mRobotEntity;
	};
}

#endif /*CHOOSEMESHWIDGET_H_*/