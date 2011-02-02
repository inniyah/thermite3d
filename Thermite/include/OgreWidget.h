#ifndef THERMITE_OGREWIDGET_H_
#define THERMITE_OGREWIDGET_H_

#include <OgreCommon.h>

#include <QWidget>

class QSettings;

namespace Thermite
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
		void initialiseOgre(const Ogre::NameValuePairList *miscParams = 0);

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

#endif /*THERMITE_OGREWIDGET_H_*/
