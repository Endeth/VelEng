#include "VWindow.h"

Vel::VWindow::VWindow() : _info("Test", glm::ivec2{ 100,100 }, glm::ivec2{ 1366, 768 }, true)
{
	InitWindow();
}

Vel::VWindow::VWindow(const VWindowInfo & info) : _info(info)
{
	InitWindow();
}

Vel::VWindow::~VWindow()
{
	glfwDestroyWindow(_window);
}

void Vel::VWindow::InitWindow()
{
	glfwWindowHint(GLFW_RESIZABLE, _info.IsResizeable());

	_window = glfwCreateWindow(_info.GetSize().x, _info.GetSize().y, _info.GetName().c_str(), nullptr, nullptr);
	assert(_window);
	glfwMakeContextCurrent(_window);
}
