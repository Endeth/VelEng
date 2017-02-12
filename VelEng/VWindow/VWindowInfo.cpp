#include "VWindowInfo.h"

using namespace std;

VWindowInfo::VWindowInfo(const string & Name, const glm::uvec2 &Position, const  glm::uvec2 &Size, bool IsResizeable) : _name(Name), _position(Position), _size(Size), _isResizeable(IsResizeable)
{
}

VWindowInfo::VWindowInfo(const string & Name, int PosX, int PosY, int Width, int Height, bool IsResizeable) : VWindowInfo(Name, { PosX, PosY }, { Width, Height }, IsResizeable)
{
}



