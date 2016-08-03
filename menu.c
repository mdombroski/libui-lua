#include "menu.h"
#include "object.h"
#include "callback.h"

#include <ui.h>


static void callback_uiMenuItemOnClicked( uiMenuItem* i, uiWindow* w, void* d )
{
	lua_State* L = (lua_State*) d;
	if( L )
	{
		object_copy( L, w );
		invoke_callback( (lua_State*) d, i, callback_OnClicked, 1 );
	}
}

DECLARE_ACTION( uiMenuItem, Enable )
DECLARE_ACTION( uiMenuItem, Disable )
DECLARE_GETTER( uiMenuItem, Checked, boolean )
DECLARE_SETTER( uiMenuItem, SetChecked, boolean )
DECLARE_CALLBACK_REGISTER( uiMenuItem, uiMenuItemSignature, OnClicked )

static luaL_Reg menuitem_functions[] =
{
	{ "Enable", l_uiMenuItemEnable },
	{ "Disable", l_uiMenuItemDisable },
	{ "Checked", l_uiMenuItemChecked },
	{ "SetChecked", l_uiMenuItemSetChecked },
	{ "OnClicked", l_uiMenuItemOnClicked },
	{ 0, 0 }
};


static int l_uiMenuAppendItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_object( L, 1, uiMenuSignature );
	uiMenuItem* i = uiMenuAppendItem( m, luaL_checkstring( L, 2 ) );
	object_create( L, i, uiMenuItemSignature, menuitem_functions, 0 );
	return 1;
}

static int l_uiMenuAppendCheckItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_object( L, 1, uiMenuSignature );
	uiMenuItem* i = uiMenuAppendCheckItem( m, luaL_checkstring( L, 2 ) );
	object_create( L, i, uiMenuItemSignature, menuitem_functions, 0 );
	return 1;
}

static int l_uiMenuAppendQuitItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_object( L, 1, uiMenuSignature );
	uiMenuItem* i = uiMenuAppendQuitItem( m );
	object_create( L, i, uiMenuItemSignature, menuitem_functions, 0 );
	return 1;
}

static int l_uiMenuAppendPreferencesItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_object( L, 1, uiMenuSignature );
	uiMenuItem* i = uiMenuAppendPreferencesItem( m );
	object_create( L, i, uiMenuItemSignature, menuitem_functions, 0 );
	return 1;
}

static int l_uiMenuAppendAboutItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_object( L, 1, uiMenuSignature );
	uiMenuItem* i = uiMenuAppendAboutItem( m );
	object_create( L, i, uiMenuItemSignature, menuitem_functions, 0 );
	return 1;
}

DECLARE_ACTION( uiMenu, AppendSeparator )

static luaL_Reg _menu_functions[] =
{
	{ "AppendItem", l_uiMenuAppendItem },
	{ "AppendCheckItem", l_uiMenuAppendCheckItem },
	{ "AppendQuitItem", l_uiMenuAppendQuitItem },
	{ "AppendPreferencesItem", l_uiMenuAppendPreferencesItem },
	{ "AppendAboutItem", l_uiMenuAppendAboutItem },
	{ "AppendSeparator", l_uiMenuAppendSeparator },
	{ 0, 0 }
};

static int new_menu( lua_State* L )
{
	void* m = uiNewMenu( luaL_optlstring( L, 1, "Menu", NULL ) );
	object_create( L, m, uiMenuSignature, _menu_functions, 0 );
	return 1;
}


luaL_Reg menu_functions[] =
{
    { "NewMenu", new_menu },
    { 0, 0 }
};
