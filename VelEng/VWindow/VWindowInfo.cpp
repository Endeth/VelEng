#include "vWindowInfo.h"

using namespace std;

WindowInfo::WindowInfo(const string & Name, const glm::uvec2 &Position, const  glm::uvec2 &Size, bool IsResizeable) : _name(Name), _position(Position), _size(Size), _isResizeable(IsResizeable)
{
}

WindowInfo::WindowInfo(const string & Name, int PosX, int PosY, int Width, int Height, bool IsResizeable) : WindowInfo(Name, { PosX, PosY }, { Width, Height }, IsResizeable)
{
}



