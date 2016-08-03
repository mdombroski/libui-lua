#include "control-common.h"
#include "object.h"

#include <ui.h>


int l_uiControlDestroy( lua_State* L )
{
	uiControl* c = check_object( L, 1, uiControlSignature );
    uiControlDestroy( c );

    // destroy registry meta table
    lua_pushlightuserdata( L, c );
    lua_pushnil( L );
    lua_settable( L, LUA_REGISTRYINDEX );

	return 0;
}

static int l_uiControlParent( lua_State* L )
{
    return object_copy( L, uiControlParent( check_object( L,-1, uiControlSignature ) ) );
}

static int l_uiControlSetParent( lua_State* L )
{
	uiControl* c = check_object( L, 1, uiControlSignature );
	uiControl* p = check_object( L, 2, uiControlSignature );
    uiControlSetParent( c, p );
	lua_pushvalue( L, 1 );
	return 1;
}

DECLARE_GETTER( uiControl, Handle, integer )
DECLARE_GETTER( uiControl, Visible, boolean )
DECLARE_ACTION( uiControl, Show )
DECLARE_ACTION( uiControl, Hide )
DECLARE_GETTER( uiControl, Enabled, boolean )
DECLARE_ACTION( uiControl, Enable )
DECLARE_ACTION( uiControl, Disable )
DECLARE_GETTER( uiControl, Toplevel, boolean )
DECLARE_GETTER( uiControl, EnabledToUser, boolean )

luaL_Reg control_common[] =
{
	{ "Destroy", l_uiControlDestroy },
	{ "Handle", l_uiControlHandle },
	{ "Parent", l_uiControlParent },
	{ "SetParent", l_uiControlSetParent },
	{ "TopLevel", l_uiControlToplevel },
	{ "Visible", l_uiControlVisible },
	{ "Show", l_uiControlShow },
	{ "Hide", l_uiControlHide },
	{ "Enabled", l_uiControlEnabled },
	{ "Enable", l_uiControlEnable },
	{ "Disable", l_uiControlDisable },
	{ "EnabledToUser", l_uiControlEnabledToUser },
	{ 0, 0 }
};
