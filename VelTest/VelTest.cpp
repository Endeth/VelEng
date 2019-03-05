// VelTest.cpp : Defines the entry point for the console application.
//

#ifdef WIN32
#include <Windows.h>
#endif
#include <functional>

#include "VelGL.h"
#include "vUti/VelEngUti.h"
#include "vGraphics/vLights/vLight.h"
#include "vGraphics/vMaterials/vMaterial.h"
#include "../VelUti/vTime.h"
#include "VelTestConfig.h"

//#include "json.hpp"

using namespace Vel;

void setVSync( bool sync )
{
#ifdef VSYNC && WIN32
    typedef BOOL( APIENTRY *PFNWGLSWAPINTERVALPROC )(int);
    PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

    if ( wglSwapIntervalEXT )
        wglSwapIntervalEXT( sync );
#endif
}


int main()
{
    VelEng::Settings initSettings;
    VelEng::Instance()->Init(initSettings);
	VelEng::Instance()->CreateScene("World");
	VelEng::Instance()->CreateScene("Sky");

	while( VelEng::Instance()->ShouldRun() )
	{
		VelEng::Instance()->GetFrameClock().Tick();
		VelEng::Instance()->HandleInput();
		VelEng::Instance()->RenderFrame();
		//VelEng::Instance()->GetFrameClock().CapFPS(); //TODO
	}

    VelEng::Destroy();

    return 0;
}
