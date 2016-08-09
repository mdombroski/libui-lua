#include "control-common.h"
#include "object.h"

#include <ui.h>


int is_control( int signature )
{
	switch( signature )
	{
	// ONLY CONTROLS/WIDGETS
	// IMPORTANT, NOT MENUS or IMAGES
	case uiAreaSignature:
	case uiBoxSignature:
	case uiButtonSignature:
	case uiCheckboxSignature:
	case uiColorButtonSignature:
	case uiComboboxSignature:
	case uiDateTimePickerSignature:
	case uiEditableComboboxSignature:
	case uiEntrySignature:
	case uiFontButtonSignature:
	case uiFormSignature:
	case uiGridSignature:
	case uiGroupSignature:
	case uiLabelSignature:
	case uiImageBoxSignature:
	case uiMultilineEntrySignature:
	case uiProgressBarSignature:
	case uiRadioButtonsSignature:
	case uiSeparatorSignature:
	case uiSliderSignature:
	case uiSpinboxSignature:
	case uiTabSignature:
	case uiWindowSignature:
		return 1;
	default:
		return 0;
	}
}

uiControl* check_control( lua_State* L, int idx, int signature )
{
	int s;
	uiControl* c = (uiControl*) get_object( L, idx, &s );

	if( is_control( s ) )
	{
		if( signature == 0 || signature == s )
		{
			return c;
		}
	}

	luaL_error( L, "libui object is not a control" );
	return 0;
}


int l_uiControlDestroy( lua_State* L )
{
	uiControl* c = check_control( L, 1, 0 );
    uiControlDestroy( c );

    // destroy registry meta table
    lua_pushlightuserdata( L, c );
    lua_pushnil( L );
    lua_settable( L, LUA_REGISTRYINDEX );

	return 0;
}

static int l_uiControlParent( lua_State* L )
{
    return object_copy( L, uiControlParent( check_control( L,-1, 0 ) ) );
}

static int l_uiControlSetParent( lua_State* L )
{
	uiControl* c = check_control( L, 1, 0 );
	uiControl* p = check_control( L, 2, 0 );
    uiControlSetParent( c, p );
	lua_pushvalue( L, 1 );
	return 1;
}

#undef DECLARE_GETTER
#define DECLARE_GETTER( typename, action, type ) \
    int l_ ## typename ## action( lua_State* L ) { \
        lua_push ## type ( L, typename ## action ( (typename*) check_control( L, 1, 0 ) ) ); \
        return 1; \
    }

#undef DECLARE_ACTION
#define DECLARE_ACTION( typename, action ) \
    int l_ ## typename ## action( lua_State* L ) { \
        typename ## action ( (typename*) check_control( L, 1, 0 ) ); \
        lua_pushvalue( L, 1 ); \
        return 1; \
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
