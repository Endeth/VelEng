#include "vWindow.h"

namespace Vel
{
    WindowInfo::WindowInfo( const std::string & Name, const glm::uvec2 & Position, const glm::uvec2 & Size, bool IsResizeable ) : _name( Name ), _position( Position ), _size( Size ), _isResizeable( IsResizeable )
    {
    }

    WindowInfo::WindowInfo( const std::string & Name, int PosX, int PosY, int Width, int Height, bool IsResizeable ) : _name( Name ), _position( PosX, PosY ), _size( Width, Height ), _isResizeable( IsResizeable )
    {
    }

    Window::Window() : _info( "Test", glm::ivec2{ 100,100 }, glm::ivec2{ 1366, 768 }, false )
    {
        InitWindow();
    }

    Window::Window( const WindowInfo & info ) : _info( info )
    {
        InitWindow();
    }

    Window::~Window()
    {
        glfwDestroyWindow( _window );
    }

    void Window::InitWindow()
    {
        glfwWindowHint( GLFW_RESIZABLE, _info.IsResizeable() );

        _window = glfwCreateWindow( _info.GetSize().x, _info.GetSize().y, _info.GetName().c_str(), nullptr, nullptr );
        assert( _window );
    }
}
