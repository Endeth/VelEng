#pragma once

extern "C"
{
#include "lua535/lua.h"
#include "lua535/lauxlib.h"
#include "lua535/lualib.h"
}

namespace Vel
{
	class Lua
	{
	public:
		Lua();
		~Lua();

		bool DoFile( const char* path );
		bool GetGlobal( const char* name );
		bool GetNumber( float& number, int stackPosition );

	private:
		bool Validate( int res );
		lua_State *luaState;
	};

}