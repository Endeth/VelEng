// VelTest.cpp : Defines the entry point for the console application.
//

#include <functional>

#include "VelGL.h"
#include "vUti/VelEngUti.h"
#include "vLights/vLight.h"
#include "vMaterials/vMaterial.h"
#include "../VelUti/vTime.h"
#include "VelTestConfig.h"

#ifdef WIN32
    #include <Windows.h>
#endif
//#include "json.hpp"

using namespace Vel;

void setVSync( bool sync )
{
#ifdef WIN32
    /*typedef BOOL( APIENTRY *PFNWGLSWAPINTERVALPROC )(int);
    PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

    if ( wglSwapIntervalEXT )
        wglSwapIntervalEXT( sync );*/
#endif
}


int main()
{
    VelEng::Settings initSettings;
    VelEng::Instance()->Init(initSettings);
	VelEng::Instance()->CreateScene("World");
	VelEng::Instance()->CreateScene("Sky");

    setVSync( false );

    return 0;
}
