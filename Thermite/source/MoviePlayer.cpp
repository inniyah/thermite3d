#include "MoviePlayer.h"

#include "Log.h"


namespace Thermite
{
	MoviePlayer::MoviePlayer(QWidget* parent, Qt::WindowFlags f)
		:QDialog(parent, f)
	{
		setupUi(this);
	}

	void MoviePlayer::setMovie(QMovie* movie)
	{
		mMovie = movie;
		mDisplay->setMovie(movie);
	}

	void MoviePlayer::handleError(QImageReader::ImageReaderError error)
	{
		thermiteGameLogic->mThermiteLog->logMessage("QMovie error occured", QtOgre::LL_ERROR);
	}
}
