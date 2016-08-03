#include "callback.h"

#include <stdio.h>


void register_callback( lua_State* L, int idx, void* sender, int callback )
{
    if( sender == NULL )
        return;

	lua_pushvalue( L, idx );
	lua_pushlightuserdata( L, sender );
	lua_pushlightuserdata( L, sender );
	lua_gettable( L, LUA_REGISTRYINDEX );

	if( lua_type( L, -1 ) != LUA_TTABLE )
	{
		lua_pop( L, 1 ); // remove nil
		lua_newtable( L );
	}

	lua_pushinteger( L, callback );
	lua_pushvalue( L, -4 );
	lua_settable( L, -3 );

	lua_settable( L, LUA_REGISTRYINDEX );
	lua_pop( L, 1 );
}

int invoke_callback( lua_State* L, void* id, int callback, int args )
{
    int err = 0;

	if( L && id )
	{
        // fetch the callback function from the registry meta data table
        lua_pushlightuserdata( L, id );
        lua_gettable( L, LUA_REGISTRYINDEX );
        lua_pushinteger( L, callback );
        lua_gettable( L, -2 );
        lua_rotate( L, -2, 1 );
        lua_pop( L, 1 );

        // not a function
        if( ! lua_isfunction( L, -1 ) )
        {
            lua_pop( L, args + 1 );
            return err;
        }

        // get object/self argument from registry (function argument)
        if( object_copy( L, id ) )
            args++;

        // make sure things are in the right order
        if( args > 1 )
            lua_rotate( L, -( args + 1 ), 2 );

		if( lua_pcall( L, args, 1, 0 ) )                   // call the callback
			printf( "%s\n", lua_tostring( L, -1 ) );    // print the error

		if( lua_isinteger( L, -1 ) )
            err = lua_tointeger( L, -1 );

        lua_pop( L, 1 );
	}

	return err;
}
