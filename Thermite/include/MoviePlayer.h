#ifndef THERMITE_MOVIEPLAYER_H_
#define THERMITE_MOVIEPLAYER_H_

#include "ui_MoviePlayer.h"

#include <QDialog>
#include <QTime>
#include <QTimer>

#include <QMovie>

#include "ThermiteGameLogic.h"

namespace Thermite
{
	class ThermiteGameLogic;

	class MoviePlayer : public QDialog, private Ui::MoviePlayer
	{
		Q_OBJECT

	public:
		MoviePlayer(QWidget* parent = 0, Qt::WindowFlags f = 0);

		void setMovie(QMovie* movie);

		QMovie* mMovie;

		ThermiteGameLogic* thermiteGameLogic;

	public slots:
		void handleError(QImageReader::ImageReaderError error);
	};
}

#endif /*THERMITE_MOVIEPLAYER_H_*/
