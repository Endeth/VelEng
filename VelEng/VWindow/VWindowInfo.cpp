#include "VWindowInfo.h"

using namespace std;

VWindowInfo::VWindowInfo(const string & Name, glm::uvec2 Position, glm::uvec2 Size, bool IsResizeable) : _name(Name), _position(Position), _size(Size), _isResizeable(IsResizeable)
{
}

VWindowInfo::VWindowInfo(const string & Name, int PosX, int PosY, int Width, int Height, bool IsResizeable) : VWindowInfo(Name, { PosX, PosY }, { Width, Height }, IsResizeable)
{
}


VWindowInfo::VWindowInfo(const VWindowInfo & Rhc) : _name(Rhc._name), _position(Rhc._position), _size(Rhc._size), _isResizeable(Rhc._isResizeable)
{

}

void VWindowInfo::operator=(const VWindowInfo & Rhc)
{
	_name = Rhc._name;
	_position = Rhc._position;
	_size = Rhc._size;
	_isResizeable = Rhc._isResizeable;
}

