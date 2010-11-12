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

#ifndef __ThermiteForwardDeclarations_H__
#define __ThermiteForwardDeclarations_H__

namespace Thermite
{
	//Resources
	class VolumeManager;
	class VolumeResourse;

	//Scriptable
	class Keyboard;
	class Mouse;
	class SkyBox;

	//Tasks
	class SurfaceMeshDecimationTask;
	class SurfaceMeshExtractionTask;
	class Task;
	class TaskProcessorThread;

	//Other
	class ApplicationGameLogic;
	class Console;
	class LoadMapWidget;
	class LoadSceneMenuPage;
	class MainMenuPage;
	
	class PhysicalObject;
	
	class SurfacePatchRenderable;
	
	class ThermiteGameLogic;
	class Volume;
	
	class VolumeSerializationProgressListener;
}

#endif
