// VelTest.cpp : Defines the entry point for the console application.
//

#include "VelEng.h"
#include <iostream>
#include "utils/TSQueue.h"
#include "utils/RenderThreadPool.h"

int main()
{
    Vel::Engine& velEng = Vel::Engine::Instance();

    velEng.Run();

    velEng.Cleanup();

    return 0;
}
