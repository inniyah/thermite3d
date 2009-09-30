#include "ApplicationGameLogic.h"

#include "Map.h"
#include "Shell.h"

namespace Thermite
{
	void ApplicationGameLogic::update(void)
	{
		//FIXME: This shold really be called at the end, so that it calls 
		//updatePolyVoxGeometry() after we've actually changed something!
		ThermiteGameLogic::update();

		if(mMap == 0)
			return;

		/*mLastFrameTime = mCurrentTime;
		mCurrentTime = mTime->elapsed();

		float timeElapsedInSeconds = (mCurrentTime - mLastFrameTime) / 1000.0f;*/

		float distance = mCameraSpeed * mTimeElapsedInSeconds;

		if(mKeyStates[Qt::Key_W] == KS_PRESSED)
		{
			mActiveCamera->setPosition(mActiveCamera->getPosition() + mActiveCamera->getDirection() * distance);
		}
		if(mKeyStates[Qt::Key_S] == KS_PRESSED)
		{
			mActiveCamera->setPosition(mActiveCamera->getPosition() - mActiveCamera->getDirection() * distance);
		}
		if(mKeyStates[Qt::Key_A] == KS_PRESSED)
		{
			mActiveCamera->setPosition(mActiveCamera->getPosition() - mActiveCamera->getRight() * distance);
		}
		if(mKeyStates[Qt::Key_D] == KS_PRESSED)
		{
			mActiveCamera->setPosition(mActiveCamera->getPosition() + mActiveCamera->getRight() * distance);
		}

		if(mCurrentFrameNumber != 0)
		{
			QPoint mouseDelta = mCurrentMousePos - mLastFrameMousePos;
			mActiveCamera->yaw(Ogre::Radian(-mouseDelta.x() * mCameraRotationalSpeed));
			mActiveCamera->pitch(Ogre::Radian(-mouseDelta.y() * mCameraRotationalSpeed));

			int wheelDelta = mCurrentWheelPos - mLastFrameWheelPos;
			Ogre::Radian fov = mActiveCamera->getFOVy();
			fov += Ogre::Radian(-wheelDelta * 0.001);
			fov = (std::min)(fov, Ogre::Radian(2.0f));
			fov = (std::max)(fov, Ogre::Radian(0.5f));
			mActiveCamera->setFOVy(fov);
		}
		mLastFrameMousePos = mCurrentMousePos;
		mLastFrameWheelPos = mCurrentWheelPos;

		//Update the cannon
		if(mTurretNode && mGunNode)
		{
			float directionInDegrees = mCannonController->direction();
			float elevationInDegrees = mCannonController->elevation();
			mTurretNode->setOrientation(mTurretOriginalOrientation);
			mGunNode->setOrientation(mGunOriginalOrientation);

			mTurretNode->rotate(Ogre::Vector3(0.0,1.0,0.0), Ogre::Radian(directionInDegrees / 57.0));
			mGunNode->rotate(Ogre::Vector3(0.0,0.0,1.0), Ogre::Radian(elevationInDegrees / 57.0)); //Elevation
		}

		//The fun stuff!
		/*updatePolyVoxGeometry();
		
#ifdef ENABLE_BULLET_PHYSICS
		if((qApp->settings()->value("Physics/SimulatePhysics", false).toBool()) && (bLoadComplete))
		{
			m_pOgreBulletWorld->stepSimulation(timeElapsedInSeconds, 10);
		}
#endif //ENABLE_BULLET_PHYSICS
*/
		list<Shell*> shellsToDelete;

		for(list<Shell*>::iterator iter = m_listShells.begin(); iter != m_listShells.end(); iter++)
		{
			(*iter)->update(mTimeElapsedInSeconds);
			Ogre::Vector3 shellPos = (*iter)->m_pSceneNode->getPosition();

			if(mMap->volumeResource->getVolume()->getEnclosingRegion().containsPoint(PolyVox::Vector3DFloat(shellPos.x, shellPos.y, shellPos.z), 1.0))
			{
				if(mMap->volumeResource->getVolume()->getVoxelAt(shellPos.x, shellPos.y, shellPos.z) != 0)
				{
					createSphereAt(PolyVox::Vector3DFloat(shellPos.x, shellPos.y, shellPos.z), 50, 0);
					shellsToDelete.push_back(*iter);
				}
			}
		}

		for(list<Shell*>::iterator iter = shellsToDelete.begin(); iter != shellsToDelete.end(); iter++)
		{
			m_listShells.remove(*iter);
			delete (*iter);
		}

		//++mCurrentFrameNumber;
	}
}