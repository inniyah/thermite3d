#ifndef QTOGRE_OGREWIDGET_H_
#define QTOGRE_OGREWIDGET_H_

#include <OgreCommon.h>

#include <QWidget>

class QSettings;

namespace QtOgre
{
	/**
	 * Widget holding Ogre
	 * 
	 * This widget is used to hold and contain the Ogre RenderWindow
	 * 
	 * \author David Williams
	 */
	class OgreWidget : public QWidget
	{
		Q_OBJECT

	public:
		OgreWidget(QWidget* parent=0, Qt::WindowFlags f=0);
		~OgreWidget();

		Ogre::RenderWindow* getOgreRenderWindow() const;

		//Other
		bool applySettings(QSettings* settings);
		void initialise(const Ogre::NameValuePairList *miscParams = 0);

	protected:
		QPaintEngine *paintEngine() const;
		void paintEvent(QPaintEvent* evt);
		void resizeEvent(QResizeEvent* evt);

	public:
		Ogre::RenderWindow* m_pOgreRenderWindow;

	private:
		bool mIsInitialised;
	};
}

#endif /*QTOGRE_OGREWIDGET_H_*/
