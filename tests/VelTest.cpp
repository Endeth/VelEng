// VelTest.cpp : Defines the entry point for the console application.
//

#include "VelEng.h"
#include <iostream>

int main()
{
    Vel::Engine velEng;

    velEng.Init();
    velEng.Run();
    velEng.Cleanup();

    return 0;
}
