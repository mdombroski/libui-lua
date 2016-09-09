#include "area.h"

#include "object.h"
#include "control-common.h"
#include "draw.h"

#include <lua.h>
#include <lauxlib.h>

#include <ui.h>

#include <stdlib.h>


static int l_uiAreaSetSize( lua_State* L )
{
	uiArea* a = (uiArea*) check_object( L, 1, uiAreaSignature );
	uiAreaSetSize( a, luaL_checkinteger( L, 2 ), luaL_checkinteger( L, 3 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_uiAreaQueueRedrawAll( lua_State* L )
{
	uiAreaQueueRedrawAll( (uiArea*) check_object( L, 1, uiAreaSignature ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_uiAreaScrollTo( lua_State* L )
{
	uiArea* a = (uiArea*) check_object( L, 1, uiAreaSignature );
	// x, y, width, height
	uiAreaScrollTo( a, luaL_checknumber( L, 2 ), luaL_checknumber( L, 3 ), luaL_checknumber( L, 4 ), luaL_checknumber( L, 5 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static luaL_Reg _area_functions[] =
{
	{ "SetSize", l_uiAreaSetSize },
	{ "QueueRedrawAll", l_uiAreaQueueRedrawAll },
	{ "ScrollTo", l_uiAreaScrollTo },
	{ 0, 0 }
};

// wow, this has lots of overhead!
static void draw( uiAreaHandler* handler, uiArea* area, uiAreaDrawParams* params )
{
	lua_State* L = *( (lua_State**)( handler + 1 ) );
	object_retreive_with_function( L, area, "Draw" );

	if( lua_isfunction( L, -2 ) )
	{
		// draw context
		draw_new_context( L, params->Context );

		// geometry
		lua_newtable( L );
		lua_pushnumber( L, params->ClipX );
		lua_setfield( L, -2, "x" );
		lua_pushnumber( L, params->ClipY );
		lua_setfield( L, -2, "y" );
		lua_pushnumber( L, params->ClipWidth );
		lua_setfield( L, -2, "w" );
		lua_pushnumber( L, params->ClipHeight );
		lua_setfield( L, -2, "h" );

		lua_call( L, 3, 0 );
	}
	else
	{
		lua_pop( L, 2 );
	}
}

static void mouseevent( uiAreaHandler* handler, uiArea* area, uiAreaMouseEvent* m )
{
	lua_State* L = *( (lua_State**)( handler + 1 ) );
	object_retreive_with_function( L, area, "MouseEvent" );
	
	if( lua_isfunction( L, -2 ) )
	{
		lua_newtable( L );
		lua_pushnumber( L, m->X );
		lua_setfield( L, -2, "x" );
		lua_pushnumber( L, m->Y );
		lua_setfield( L, -2, "y" );
		lua_pushnumber( L, m->AreaWidth );
		lua_setfield( L, -2, "w" );
		lua_pushnumber( L, m->AreaHeight );
		lua_setfield( L, -2, "h" );
		lua_pushnumber( L, m->Down );
		lua_setfield( L, -2, "down" );
		lua_pushnumber( L, m->Up );
		lua_setfield( L, -2, "up" );
		lua_pushnumber( L, m->Count );
		lua_setfield( L, -2, "count" );

		lua_pushnumber( L, m->Modifiers );
		lua_setfield( L, -2, "modifiers" );
		lua_pushnumber( L, m->Held1To64 );
		lua_setfield( L, -2, "held1to64" );

		lua_call( L, 2, 0 );
	}
	else
	{
		lua_pop( L, 2 );
	}
}

static void mousecrossed( uiAreaHandler* handler, uiArea* area, int left )
{
	lua_State* L = *( (lua_State**)( handler + 1 ) );
	object_retreive_with_function( L, area, "MouseCrossed" );
	
	if( lua_isfunction( L, -2 ) )
	{
		lua_pushboolean( L, left );
		lua_call( L, 2, 0 );
	}
	else
	{
		lua_pop( L, 2 );
	}
}

static void dragbroken( uiAreaHandler* handler, uiArea* area )
{
	lua_State* L = *( (lua_State**)( handler + 1 ) );
	object_retreive_with_function( L, area, "DragBroken" );
	
	if( lua_isfunction( L, -2 ) )
	{
		lua_call( L, 1, 0 );
	}
	else
	{
		lua_pop( L, 2 );
	}
}

static int keyevent( uiAreaHandler* handler, uiArea* area, uiAreaKeyEvent* k )
{
	lua_State* L = *( (lua_State**)( handler + 1 ) );
	object_retreive_with_function( L, area, "KeyEvent" );
	
	if( lua_isfunction( L, -2 ) )
	{
		lua_newtable( L );
		lua_pushnumber( L, k->Key );
		lua_setfield( L, -2, "key" );
		lua_pushnumber( L, k->ExtKey );
		lua_setfield( L, -2, "extkey" );
		lua_pushnumber( L, k->Modifier );
		lua_setfield( L, -2, "modifier" );
		lua_pushnumber( L, k->Modifiers );
		lua_setfield( L, -2, "modifiers" );
		lua_pushnumber( L, k->Up );
		lua_setfield( L, -2, "up" );

		lua_call( L, 2, 0 );
	}
	else
	{
		lua_pop( L, 2 );
	}
	return 0;
}


static int new_area( lua_State* L )
{
	// TODO
	uiArea* a;
	uiAreaHandler* ah = malloc( sizeof(uiAreaHandler) + sizeof(lua_State*) );
	ah->Draw = draw;
	ah->MouseEvent = mouseevent;
	ah->MouseCrossed = mousecrossed;
	ah->DragBroken = dragbroken;
	ah->KeyEvent = keyevent;
	*( (lua_State**)( ah + 1 ) ) = L;

	if( lua_isinteger( L, 2 ) && lua_isinteger( L, 3 ) )
	{
		a = uiNewScrollingArea( ah, lua_tointeger( L, 2 ), lua_tointeger( L, 3 ) );
	}
	else
	{
		a = uiNewArea( ah );
	}

	object_create( L, a, uiAreaSignature, control_common, _area_functions, 0 );
	return 1;
}

luaL_Reg area_functions[] =
{
	{ "NewArea", new_area },
	{ 0, 0 }
};
