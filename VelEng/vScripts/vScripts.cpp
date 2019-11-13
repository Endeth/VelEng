#include "vScripts.h"
#include <string>
#include <iostream>

Vel::Lua::Lua()
{
	luaState = luaL_newstate();
	luaL_openlibs( luaState );
}

Vel::Lua::~Lua()
{
	lua_close( luaState );
}

bool Vel::Lua::DoFile( const char* path )
{
	return Validate( luaL_dofile( luaState, path ) );
}

bool Vel::Lua::GetGlobal( const char* name )
{
	lua_getglobal( luaState, name );
	return lua_isnil( luaState, -1 );
}

bool Vel::Lua::GetNumber( float& number, int stackPosition )
{
	if( lua_isnumber( luaState, stackPosition ) )
	{
		number = (float)lua_tonumber( luaState, stackPosition );
	}
	return false;
}

bool Vel::Lua::Validate( int res )
{
	if( res != LUA_OK )
	{
		std::string errorMsg = lua_tostring( luaState, -1 );
		std::cout << errorMsg << std::endl;

		return false;
	}
	return true;
}
