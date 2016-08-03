#include "object.h"

#include <stdarg.h>


int object_gc( lua_State* L )
{
    if( lua_type( L, 1 ) != LUA_TUSERDATA )
    {
        luaL_error( L, "libui not a userdata" );
        return 0;
    }

    int object_signature = 0;

    // fetch object signature from its metatable data
    lua_getmetatable( L, 1 );
    if( lua_istable( L, -1 ) )
    {
        lua_pushstring( L, "__libui_signature" );
        lua_gettable( L, -2 );
        if( lua_isinteger( L, -1 ) )
        {
            object_signature = lua_tointeger( L, -1 );
        }
        lua_pop( L, 1 );
    }
    lua_pop( L, 1 );

    if( object_signature == 0 )
    {
        luaL_error( L, "libui not an object" );
        return 0;
    }
    if( object_signature == uiMenuSignature || object_signature == uiMenuItemSignature )
    {
        // can't delete anything from menus
        return 0;
    }

    // TODO maybe one day when libui gets an OnDestroy callback this can be implemented fully.

    return 0;
}

// NOTE: when calling object_create, the last argument MUST be 0 (just add it on the end)
void* object_create( lua_State* L, void* object, int signature, ... )
{
    // userdata wrapper is a single pointer
    void** p = lua_newuserdata( L, sizeof(object) );
    *p = object;

	// create a metatable
	lua_newtable( L );

	// copy control signature to metatable
    lua_pushinteger( L, signature );
    lua_setfield( L, -2, "__libui_signature" );

    // variable list of meta functions
    va_list ap;
    va_start( ap, signature );
    while( 1 )
    {
        luaL_Reg* fn = va_arg( ap, luaL_Reg* );
        if( fn )
            luaL_setfuncs( L, fn, 0 );
        else
            break;
    }
    va_end(ap);

    lua_pushcfunction( L, object_gc );
    lua_setfield( L, -2, "__gc" );

	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );

	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__newindex" );

    // create meta data table in registry
    lua_pushlightuserdata( L, object );
    lua_newtable( L );
    lua_pushvalue( L, -3 );
    lua_setfield( L, -2, "meta" );
    lua_settable( L, LUA_REGISTRYINDEX );

	lua_setmetatable( L, -2 );
	
	return object;
}

int object_copy( lua_State* L, void* object )
{
    // retreive meta table from registry
    lua_pushlightuserdata( L, object );
    lua_gettable( L, LUA_REGISTRYINDEX );

    // object not registered, can't continue
    if( ! lua_istable( L, -1 ) )
    {
        lua_pop( L, 1 );
        return 0;
    }

    // get function table, pop previous table
    lua_getfield( L, -1, "meta" );
    lua_rotate( L, -2, 1 );
    lua_pop( L, 1 );

    // meta table not valid, can't continue
    if( ! lua_istable( L, -1 ) )
    {
        lua_pop( L, 1 );
        return 0;
    }

    void** p = lua_newuserdata( L, sizeof(object) );
    *p = object;

    lua_rotate( L, -2, 1 );
    lua_setmetatable( L, -2 );

    return 1;
}

void* check_object( lua_State* L, int idx, int signature )
{
    if( lua_type( L, idx ) != LUA_TUSERDATA )
    {
        luaL_error( L, "libui not a userdata" );
        return 0;
    }

    int object_signature = 0;

    // fetch object signature from its metatable data
    lua_getmetatable( L, idx );
    if( lua_istable( L, -1 ) )
    {
        lua_pushstring( L, "__libui_signature" );
        lua_gettable( L, -2 );
        if( lua_isinteger( L, -1 ) )
        {
            object_signature = lua_tointeger( L, -1 );
        }
        lua_pop( L, 1 );
    }
    lua_pop( L, 1 );

    void** p = lua_touserdata( L, idx );
    if( p == NULL || *p == NULL )
    {
        luaL_error( L, "libui object pointer invalid (NULL)" );
        return 0;
	}

	void* o = *p;

	// check object is valid in registry
	lua_pushlightuserdata( L, o );
	lua_gettable( L, LUA_REGISTRYINDEX );
	if( lua_isnil( L, -1 ) )
    {
        luaL_error( L, "libui object is not valid in registry (it might have been deleted)" );
        return 0;
    }
    lua_pop( L, 1 );

	// match explicit signature
    if( object_signature == signature )
    {
        return o;
    }

    // uiMenu and uiMenuItem objects are not uiControl objects
    if( uiControlSignature == signature && ( uiMenuSignature != signature && uiMenuItemSignature != signature ) )
    {
        return o;
    }

    luaL_error( L, "libui object signature mismatch" );
	return 0;
}

