#include <lua.h>
#include <lauxlib.h>

#include <ui.h>

#include "object.h"
#include "callback.h"
#include "queue.h"
#include "control-common.h"
#include "menu.h"
#include "controls.h"

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


static luaL_Reg area_functions[] =
{
	{ 0, 0 }
};

static int new_area( lua_State* L )
{
	// TODO
	uiArea* a;
	uiAreaHandler* ah = 0; // required
	if( lua_isinteger( L, 2 ) && lua_isinteger( L, 3 ) )
	{
		a = uiNewScrollingArea( ah, lua_tointeger( L, 2 ), lua_tointeger( L, 3 ) );
	}
	else
	{
		a = uiNewArea( ah );
	}

	object_create( L, a, uiAreaSignature, control_common, area_functions, 0 );
	return 1;
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
	luaL_setfuncs( L, controls_functions, 0 );
	luaL_setfuncs( L, menu_functions, 0 );

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
