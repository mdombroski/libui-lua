#include <lua.h>
#include <lauxlib.h>

#include <ui.h>

#include "object.h"
#include "callback.h"
#include "control-common.h"
#include "menu.h"
#include "controls.h"
#include "image.h"
#include "area.h"
#include "draw.h"

#ifndef MODULE_API
#define MODULE_API __attribute__((visibility("default")))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static uiInitOptions init_options;



static int l_uiMain( lua_State* L )
{
	uiMain();
	return 0;
}

static int l_uiQuit( lua_State* L )
{
	uiQuit();
	return 0;
}

typedef struct timeout_data
{
	lua_State* L;
} timeout_data;

static int invoke_timeout( void* data )
{
	timeout_data *td = (timeout_data *) data;
	lua_State* L = td->L;

	lua_pushlightuserdata( L, td );
	lua_gettable( L, LUA_REGISTRYINDEX );

	if (lua_isfunction( L, -1 ) )
	{
		lua_call( L, 0, 1 );

		int status = lua_toboolean( L, -1 );
		lua_pop( L, 1 );
		if (status)
			// reschedule
			return 1;
	}

	// delete timeout data
	lua_pushnil( L );
	lua_settable( L, LUA_REGISTRYINDEX );
	free( td );

	// do not reschedule
	return 0;
}

static int l_uiTimeout( lua_State* L )
{
	int timeout = luaL_checkinteger( L, 1 );
	luaL_checktype( L, 2, LUA_TFUNCTION );

	timeout_data *data = (timeout_data *) malloc( sizeof(timeout_data) );
	data->L = L;

	// REGISTRY[lightuserdata] = function
	lua_pushlightuserdata( L, data );
	lua_pushvalue( L, 2 );
	lua_settable( L, LUA_REGISTRYINDEX );

	uiTimeout( timeout, invoke_timeout, data );

	return 0;
}

static int l_uiQueueMain( lua_State* L )
{
	printf( "STUB uiQueueMain\n" );
	return 0;
}

static int callback_uiShouldQuit( void* data )
{
	return invoke_callback( (lua_State*) data, &init_options, callback_ShouldQuit, 0 );
}

static int l_uiOnShouldQuit( lua_State* L )
{
	register_callback( L, -1, &init_options, callback_ShouldQuit );
	if( lua_isfunction( L, -1 ) )
		uiOnShouldQuit( callback_uiShouldQuit, L );

	return 0;
}


static int openfile( lua_State* L )
{
    char* f = uiOpenFile( (uiWindow*) check_object( L, 1, uiWindowSignature ) );
    if( f )
	{
		lua_pushstring( L, f );
		uiFreeText( f );
	}
	else
    {
        lua_pushnil( L );
    }
    return 1;
}

static int savefile( lua_State* L )
{
    char* f = uiSaveFile( (uiWindow*) check_object( L, 1, uiWindowSignature ) );
    if( f )
    {
        lua_pushstring( L, f );
        uiFreeText( f );
    }
    else
    {
        lua_pushnil( L );
    }
    return 1;
}

static int msgbox( lua_State* L )
{
    uiWindow* w = (uiWindow*) check_object( L, 1, uiWindowSignature );
    uiMsgBox( w, luaL_checkstring( L, 2 ), luaL_checkstring( L, 3 ) );
	return 0;
}

static int msgboxerror( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_object( L, 1, uiWindowSignature );
    uiMsgBoxError( w, luaL_checkstring( L, 2 ), luaL_checkstring( L, 3 ) );
	return 0;
}

luaL_Reg ui_functions[] =
{
	{ "Main", l_uiMain },
	{ "Quit", l_uiQuit },
	{ "Timeout", l_uiTimeout },
	{ "QueueMain", l_uiQueueMain },
	{ "ShouldQuit", l_uiOnShouldQuit },

	{ "OpenFile", openfile },
	{ "SaveFile", savefile },
	{ "MsgBox", msgbox },
	{ "MsgBoxError", msgboxerror },

	{ 0, 0 }
};


MODULE_API int luaopen_libui_core( lua_State* L )
{
	char const* err;

	memset( &init_options, 0, sizeof(init_options) );
	err = uiInit( &init_options );

	if( err )
	{
		printf( "error\n" );
		uiFreeInitError( err );
		return 0;
	}

	luaL_newlib( L, ui_functions );
	luaL_setfuncs( L, image_functions, 0 );
	luaL_setfuncs( L, controls_functions, 0 );
	luaL_setfuncs( L, menu_functions, 0 );
	luaL_setfuncs( L, area_functions, 0 );

	draw_init( L );

	// these are the alignment/positioning enums for uiGrid
	lua_pushinteger( L, uiAlignFill );
	lua_setfield( L, -2, "AlignFill" );
	lua_pushinteger( L, uiAlignStart );
	lua_setfield( L, -2, "AlignStart" );
	lua_pushinteger( L, uiAlignCenter );
	lua_setfield( L, -2, "AlignCenter" );
	lua_pushinteger( L, uiAlignEnd );
	lua_setfield( L, -2, "AlignEnd" );
	lua_pushinteger( L, uiAtLeading );
	lua_setfield( L, -2, "AtLeading" );
	lua_pushinteger( L, uiAtTop );
	lua_setfield( L, -2, "AtTop" );
	lua_pushinteger( L, uiAtTrailing );
	lua_setfield( L, -2, "AtTrailing" );
	lua_pushinteger( L, uiAtBottom );
	lua_setfield( L, -2, "AtBottom" );

	return 1;
}
