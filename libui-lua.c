#include <lua.h>
#include <lauxlib.h>

// simplifies getters/setters
#define luaL_checkboolean( L, i ) lua_toboolean( L, i )

#include <ui.h>

#ifndef MODULE_API
#define MODULE_API __attribute__((visibility("default")))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// contained in platform specific file (libui-windows-lua.c or similar)
extern int open_platform_specific( lua_State* L );

static uiInitOptions init_options;


enum
{
    control_uiControl = 0,
	control_uiWindow = 0xAA1,
	control_uiButton = 0xAA2,
	control_uiBox = 0xAA3,
	control_uiCheckbox = 0xAA4,
	control_uiEntry = 0xAA5,
	control_uiLabel = 0xAA6,
	control_uiTab = 0xAA7,
	control_uiGroup = 0xAA8,
	control_uiSpinbox = 0xAA9,
	control_uiProgressBar = 0xAAA,
	control_uiSlider = 0xAAB,
	control_uiSeparator = 0xAAC,
	control_uiCombobox = 0xAAD,
	control_uiRadioButtons = 0xAAE,
	control_uiDateTimePicker = 0xAAF,
	control_uiMultilineEntry = 0xAB0,
	control_uiMenu = 0xAB1,
	control_uiMenuItem = 0xAB2,
	control_uiArea = 0xAB3,
	control_uiFontButton = 0xAB4,
	control_uiColorButton = 0xAB5,

	callback_shouldquit = 0xDA0,

	callback_ShouldQuit = 0xDA0,
	callback_OnClosing = 0xDA1,
	callback_OnClicked = 0xDA2,
	callback_OnToggled = 0xDA3,
	callback_OnChanged = 0xDA4,
	callback_OnSelected = 0xDA5,
};


static void register_callback( lua_State* L, int idx, void* id, int type )
{
	lua_pushvalue( L, idx );
	lua_pushlightuserdata( L, id );
	lua_pushlightuserdata( L, id );
	lua_gettable( L, LUA_REGISTRYINDEX );

	if( lua_type( L, -1 ) != LUA_TTABLE )
	{
		lua_pop( L, 1 ); // remove nil
		lua_newtable( L );
	}

	lua_pushinteger( L, type );
	lua_pushvalue( L, -4 );
	lua_settable( L, -3 );

	lua_settable( L, LUA_REGISTRYINDEX );
	lua_pop( L, 1 );
}

static void invoke_callback( lua_State* L, void* id, int type )
{
	if( L )
	{
	    lua_pushlightuserdata( L, id );
		lua_gettable( L, LUA_REGISTRYINDEX );
		lua_pushinteger( L, type );
		lua_gettable( L, -2 );

		if( lua_pcall( L, 0, 0, 0 ) )
		{
			printf( "%s\n", lua_tostring( L, -1 ) );
			lua_pop( L, 1 );
		}
	}
}

#define DECLARE_CALLBACK_1( typename, action ) \
    static void callback_ ## typename ## action( typename *c, void* data ) { \
        invoke_callback( (lua_State*) data, c, callback_ ## action ); \
    }
#define DECLARE_CALLBACK_2( typename, action ) \
    static int l_ ## typename ## action( lua_State* L ) { \
        typename* c = (typename*) check_control( L, 1, control_ ## typename ); \
        if( c ) \
        { \
            register_callback( L, 2, c, callback_ ## action ); \
            if( lua_type( L, 2 ) == LUA_TFUNCTION ) \
            { \
                typename ## action( c, callback_ ## typename ## action, L ); \
            } \
            else \
            { \
                typename ## action( c, NULL, NULL ); \
            } \
            lua_pushvalue( L, 1 ); \
            return 1; \
        } \
        return 0; \
    }
#define DECLARE_CALLBACK( typename, action ) \
    DECLARE_CALLBACK_1( typename, action ) \
    DECLARE_CALLBACK_2( typename, action )

#define DECLARE_SETTER( typename, action, type ) \
    int l_ ## typename ## action( lua_State* L ) { \
        typename* c = (typename*) check_control( L, 1, control_ ## typename ); \
        if( c ) { \
            typename ## action( c, luaL_check ## type ( L, 2 ) ); \
            lua_pushvalue( L, 1 ); \
            return 1; \
        } \
        return 0; \
    }

#define DECLARE_GETTER( typename, action, type ) \
    int l_ ## typename ## action( lua_State* L ) { \
        typename* c = (typename*) check_control( L, 1, control_ ## typename ); \
        if( c ) { \
            lua_push ## type ( L, typename ## action ( c ) ); \
            return 1; \
        } \
        return 0; \
    }

#define DECLARE_ACTION( typename, action ) \
    int l_ ## typename ## action( lua_State* L ) { \
        typename* c = (typename*) check_control( L, 1, control_ ## typename ); \
        if( c ) { \
            typename ## action ( c ); \
            lua_pushvalue( L, 1 ); \
        } \
        return 0; \
    }

static int uimain( lua_State* L )
{
	uiMain();
	return 0;
}

static int uiquit( lua_State* L )
{
	uiQuit();
	return 0;
}

static int queuemain( lua_State* L )
{
	printf( "STUB uiQueueMain\n" );
	return 0;
}

static int shouldquit_callback( void* data )
{
	invoke_callback( (lua_State*) data, &init_options, callback_shouldquit );
	return 0;
}

static int onshouldquit( lua_State* L )
{
	register_callback( L, -1, &init_options, callback_shouldquit );
	if( lua_isfunction( L, -1 ) )
	{
		uiOnShouldQuit( shouldquit_callback, L );
	}
	else
	{
		uiOnShouldQuit( NULL, NULL );
	}

	return 0;
}


struct control_userdata
{
	uiControl* control;
	int type;
};


static uiControl* check_control( lua_State* L, int idx, int type )
{
    if( lua_type( L, idx ) != LUA_TUSERDATA )
    {
        return 0;
    }

	struct control_userdata* c = lua_touserdata( L, idx );
	if( c )
	{
		if( c->type == type || type == 0 )
		{
			return c->control;
		}
	}

	return 0;
}


static int control_destroy( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	if( c )
	{
		// TODO destroyed controls
		// uiControlDestroy( c );
		printf( "STUB uiControlDestroy\n" );
	}
	return 0;
}

DECLARE_GETTER( uiControl, Handle, integer )

static int control_parent( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	if( c )
	{
		uiControl* p = uiControlParent( c );
		if( p )
		{
			printf( "STUB uiControlParent\n" );
			// TODO push copy of / reference to control
			return 0;
		}
	}

	return 0;
}

static int control_setparent( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	uiControl* p = check_control( L, -2, 0 );
	if( c )
	{
		uiControlSetParent( c, p );
	}

	return 0;
}

static int control_toplevel( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	if( c )
	{
		printf( "STUB uiControlTopLevel\n" );

		//uiControl* t = uiControlToplevel( c );
		//if( t )
		{
			// TODO push copy of / reference to control
			return 0;
		}
	}

	return 0;
}

DECLARE_GETTER( uiControl, Visible, boolean )
DECLARE_ACTION( uiControl, Show )
DECLARE_ACTION( uiControl, Hide )
DECLARE_GETTER( uiControl, Enabled, boolean )
DECLARE_ACTION( uiControl, Enable )
DECLARE_ACTION( uiControl, Disable )

static int control_free( lua_State* L )
{
	printf( "STUB uiFreeControl\n" );
	return 0;
}

static int control_verifydestroy( lua_State* L )
{
	printf( "STUB uiControlVerifyDestroy\n" );
	return 0;
}

static int control_verifysetparent( lua_State* L )
{
	printf( "STUB uiControlVerifySetParent\n" );
	return 0;
}

static int control_enabledtouser( lua_State* L )
{
	printf( "STUB uiControlEnabledToUser\n" );
	return 0;
}

luaL_Reg control_common[] =
{
	{ "Destroy", control_destroy },
	{ "Handle", l_uiControlHandle },
	{ "Parent", control_parent },
	{ "SetParent", control_setparent },
	{ "TopLevel", control_toplevel },
	{ "Visible", l_uiControlVisible },
	{ "Show", l_uiControlShow },
	{ "Hide", l_uiControlHide },
	{ "Enabled", l_uiControlEnabled },
	{ "Enable", l_uiControlEnable },
	{ "Disable", l_uiControlDisable },
	{ "Free", control_free },
	{ "VerifyDestroy", control_verifydestroy },
	{ "VerifySetParent", control_verifysetparent },
	{ "EnabledToUser", control_enabledtouser },
	{ 0, 0 }
};


static void control_add_methods( lua_State* L, int idx, luaL_Reg* table )
{
	lua_pushvalue( L, idx );
	luaL_setfuncs( L, table, 0 );
	lua_pop( L, 1 );
}


static void control_create( lua_State* L, uiControl* c, int type, luaL_Reg* functions )
{
	struct control_userdata* dat = lua_newuserdata( L, sizeof(struct control_userdata) );
	dat->control = (uiControl*) c;
	dat->type = type;

	lua_newtable( L );
	control_add_methods( L, -1, control_common );
	if( functions )
	{
		control_add_methods( L, -1, functions );
	}

	lua_pushstring( L, "__index" );
	lua_pushvalue( L, -2 );
	lua_settable( L, -3 );

	lua_pushstring( L, "__newindex" );
	lua_pushvalue( L, -2 );
	lua_settable( L, -3 );

	lua_setmetatable( L, -2 );
}

static int callback_uiWindowOnClosing( uiWindow* w, void* d )
{
    invoke_callback( (lua_State*) d, w, callback_OnClosing );
    return 0;
}

DECLARE_GETTER( uiWindow, Title, string )
DECLARE_SETTER( uiWindow, SetTitle, string )
DECLARE_GETTER( uiWindow, Margined, boolean)
DECLARE_SETTER( uiWindow, SetMargined, boolean )
DECLARE_CALLBACK_2( uiWindow, OnClosing )

static int window_setchild( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_uiWindow );
	uiControl* c = (uiControl*) check_control( L, 2, control_uiControl );
	if( w && c && ( uiControl(w) != c ) )
	{
		uiWindowSetChild( w, c );
	}
	return 0;
}


static luaL_Reg window_functions[] =
{
	{ "Title", l_uiWindowTitle },
	{ "SetTitle", l_uiWindowSetTitle },
	{ "Margined", l_uiWindowMargined },
	{ "SetMargined", l_uiWindowSetMargined },
	{ "SetChild", window_setchild },
	{ "OnClosing", l_uiWindowOnClosing },
	{ 0, 0 }
};

static int new_window( lua_State* L )
{
	uiWindow* w = uiNewWindow( luaL_optlstring( L, 1, "Lua", NULL ), luaL_optinteger( L, 2, 640 ), luaL_optinteger( L, 3, 480 ), lua_toboolean( L, 4 ) );
	control_create( L, (uiControl*) w, control_uiWindow, window_functions );

	return 1;
}


DECLARE_GETTER( uiButton, Text, string )
DECLARE_SETTER( uiButton, SetText, string )
DECLARE_CALLBACK( uiButton, OnClicked )

static luaL_Reg button_functions[] =
{
	{ "Text", l_uiButtonText },
	{ "SetText", l_uiButtonSetText },
	{ "OnClicked", l_uiButtonOnClicked },
	{ 0, 0 }
};

static int new_button( lua_State* L )
{
	uiButton* b = uiNewButton( luaL_optlstring( L, 1, "Button", NULL ) );
	control_create( L, (uiControl*) b, control_uiButton, button_functions );

	return 1;
}


static int box_append( lua_State* L )
{
	uiBox* b = (uiBox*) check_control( L, 1, control_uiBox );
	uiControl* o = (uiControl*) check_control( L, 2, control_uiControl );
	if( b && o )
	{
		uiBoxAppend( b, o, lua_toboolean( L, 3 ) );
		lua_pushvalue( L, 1 );
		return 1;
	}
	return 0;
}

DECLARE_GETTER( uiBox, Padded, boolean )
DECLARE_SETTER( uiBox, SetPadded, boolean )
DECLARE_SETTER( uiBox, Delete, integer )

luaL_Reg box_functions[] =
{
	{ "Append", box_append },
	{ "Delete", l_uiBoxDelete },
	{ "Padded", l_uiBoxPadded },
	{ "SetPadded", l_uiBoxSetPadded },
	{ 0, 0 }
};

static int new_hbox( lua_State* L )
{
	control_create( L, (uiControl*) uiNewHorizontalBox(), control_uiBox, box_functions );
	return 1;
}

static int new_vbox( lua_State* L )
{
	control_create( L, (uiControl*) uiNewVerticalBox(), control_uiBox, box_functions );
	return 1;
}


DECLARE_GETTER( uiCheckbox, Text, string )
DECLARE_SETTER( uiCheckbox, SetText, string )
DECLARE_GETTER( uiCheckbox, Checked, boolean )
DECLARE_SETTER( uiCheckbox, SetChecked, boolean )
DECLARE_CALLBACK( uiCheckbox, OnToggled )

luaL_Reg checkbox_functions[] =
{
	{ "Text", l_uiCheckboxText },
	{ "SetText", l_uiCheckboxSetText },
	{ "OnToggled", l_uiCheckboxOnToggled },
	{ "Checked", l_uiCheckboxChecked },
	{ "SetChecked", l_uiCheckboxSetChecked },
	{ 0, 0 }
};

static int new_checkbox( lua_State* L )
{
	control_create( L, (uiControl*) uiNewCheckbox( luaL_optlstring( L, 1, "Checkbox", NULL ) ), control_uiCheckbox, checkbox_functions );
	return 1;
}


DECLARE_GETTER( uiEntry, Text, string )
DECLARE_SETTER( uiEntry, SetText, string )
DECLARE_GETTER( uiEntry, ReadOnly, boolean )
DECLARE_SETTER( uiEntry, SetReadOnly, boolean )
DECLARE_CALLBACK( uiEntry, OnChanged )

luaL_Reg entry_functions[] =
{
	{ "Text", l_uiEntryText },
	{ "SetText", l_uiEntrySetText },
	{ "OnChanged", l_uiEntryOnChanged },
	{ "ReadOnly", l_uiEntryReadOnly },
	{ "SetReadOnly", l_uiEntrySetReadOnly },
	{ 0, 0 }
};

static int new_entry( lua_State* L )
{
	control_create( L, (uiControl*) uiNewEntry(), control_uiEntry, entry_functions );
	return 1;
}


DECLARE_GETTER( uiLabel, Text, string )
DECLARE_SETTER( uiLabel, SetText, string )

luaL_Reg label_functions[] =
{
	{ "Text", l_uiLabelText },
	{ "SetText", l_uiLabelSetText },
	{ 0, 0 }
};

static int new_label( lua_State* L )
{
	control_create( L, (uiControl*) uiNewLabel( luaL_optlstring( L, 1, "Label", NULL ) ), control_uiLabel, label_functions );
	return 1;
}


static int tab_append( lua_State* L )
{
	uiTab* t = (uiTab*) check_control( L, 1, control_uiTab );
	uiControl* c = check_control( L, 3, control_uiControl );

	if( t && c )
	{
		uiTabAppend( t, luaL_checkstring( L, 2 ), c );
	}

	return 0;
}

static int tab_insert( lua_State* L )
{
	uiTab* t = (uiTab*) check_control( L, 1, control_uiTab );
	uiControl* c = check_control( L, 4, control_uiControl );

	if( t && c )
	{
		uiTabInsertAt( t, luaL_checkstring( L, 3 ), luaL_checkinteger( L, 2 ), c );
	}

	return 0;
}

DECLARE_SETTER( uiTab, Delete, integer )
DECLARE_GETTER( uiTab, NumPages, integer )

static int l_uiTabMargined( lua_State* L )
{
	uiTab* t = (uiTab*) check_control( L, 1, control_uiTab );
	if( t )
	{
		lua_pushboolean( L, uiTabMargined( t, luaL_checkinteger( L, 2 ) ) );
	}
	return 0;
}

static int l_uiTabSetMargined( lua_State* L )
{
	uiTab* t = (uiTab*) check_control( L, 1, control_uiTab );
	if( t )
	{
		uiTabSetMargined( t, luaL_checkinteger( L, 2 ), lua_toboolean( L, 3 ) );
		lua_pushvalue( L, 1 );
		return 1;
	}
	return 0;
}

static luaL_Reg tab_functions[] =
{
	{ "Append", tab_append },
	{ "Insert", tab_insert },
	{ "Delete", l_uiTabDelete },
	{ "NumPages", l_uiTabNumPages },
	{ "Margined", l_uiTabMargined },
	{ "SetMargined", l_uiTabSetMargined },
	{ 0, 0 }
};

static int new_tab( lua_State* L )
{
	control_create( L, (uiControl*) uiNewTab(), control_uiTab, tab_functions );
	return 1;
}


DECLARE_GETTER( uiGroup, Title, string )
DECLARE_SETTER( uiGroup, SetTitle, string )
DECLARE_GETTER( uiGroup, Margined, boolean )
DECLARE_SETTER( uiGroup, SetMargined, boolean )

static int group_setchild( lua_State* L )
{
	uiGroup* g = (uiGroup*) check_control( L, 1, control_uiGroup );
	uiControl* c = check_control( L, 2, control_uiControl );
	if( g && c )
	{
		uiGroupSetChild( g, c );
	}
	return 0;
}

static luaL_Reg group_functions[] =
{
	{ "Title", l_uiGroupTitle },
	{ "SetTitle", l_uiGroupSetTitle },
	{ "SetChild", group_setchild },
	{ "Margined", l_uiGroupMargined },
	{ "SetMargined", l_uiGroupSetMargined },
	{ 0, 0 }
};

static int new_group( lua_State* L )
{
	control_create( L, (uiControl*) uiNewGroup( luaL_optlstring( L, 1, "Label", NULL ) ), control_uiGroup, group_functions );
	return 1;
}


DECLARE_GETTER( uiSpinbox, Value, integer )
DECLARE_SETTER( uiSpinbox, SetValue, integer )
DECLARE_CALLBACK( uiSpinbox, OnChanged )

static luaL_Reg spinbox_functions[] =
{
	{ "Value", l_uiSpinboxValue },
	{ "SetValue", l_uiSpinboxSetValue },
	{ "OnChanged", l_uiSpinboxOnChanged },
	{ 0, 0 }
};

static int new_spinbox( lua_State* L )
{
	control_create( L, (uiControl*) uiNewSpinbox( luaL_optinteger( L, 1, 0 ), luaL_optinteger( L, 2, 100 ) ), control_uiSpinbox, spinbox_functions );
	return 1;
}


DECLARE_SETTER( uiProgressBar, SetValue, integer )

static luaL_Reg progress_functions[] =
{
	{ "SetValue", l_uiProgressBarSetValue },
	{ 0, 0 }
};

static int new_progress( lua_State* L )
{
	control_create( L, (uiControl*) uiNewProgressBar(), control_uiProgressBar, progress_functions );
	return 1;
}


DECLARE_GETTER( uiSlider, Value, integer )
DECLARE_SETTER( uiSlider, SetValue, integer )
DECLARE_CALLBACK( uiSlider, OnChanged )

static luaL_Reg slider_functions[] =
{
	{ "Value", l_uiSliderValue },
	{ "SetValue", l_uiSliderSetValue },
	{ "OnChanged", l_uiSliderOnChanged },
	{ 0, 0 }
};

static int new_slider( lua_State* L )
{
	control_create( L, (uiControl*) uiNewSlider( luaL_optinteger( L, 1, 0 ), luaL_optinteger( L, 2, 100 ) ), control_uiSlider, slider_functions );
	return 1;
}


static int new_hseparator( lua_State* L )
{
	control_create( L, (uiControl*) uiNewHorizontalSeparator(), control_uiSeparator, 0 );
	return 1;
}


DECLARE_SETTER( uiCombobox, Append, string )
DECLARE_GETTER( uiCombobox, Selected, integer )
DECLARE_SETTER( uiCombobox, SetSelected, integer )
DECLARE_CALLBACK( uiCombobox, OnSelected )

static luaL_Reg combobox_functions[] =
{
	{ "Append", l_uiComboboxAppend },
	{ "Selected", l_uiComboboxSelected },
	{ "SetSelected", l_uiComboboxSetSelected },
	{ "OnSelected", l_uiComboboxOnSelected },
	{ 0, 0 }
};

static int new_combobox( lua_State* L )
{
	if( lua_toboolean( L, 1 ) == 1 )
	{
		control_create( L, (uiControl*) uiNewEditableCombobox(), control_uiCombobox, combobox_functions );
	}
	else
	{
		control_create( L, (uiControl*) uiNewCombobox(), control_uiCombobox, combobox_functions );
	}
	return 1;
}


DECLARE_SETTER( uiRadioButtons, Append, string )

static luaL_Reg radiobuttons_functions[] =
{
	{ "Append", l_uiRadioButtonsAppend },
	{ 0, 0 }
};

static int new_radiobuttons( lua_State* L )
{
	control_create( L, (uiControl*) uiNewRadioButtons(), control_uiRadioButtons, radiobuttons_functions );
	return 1;
}


static luaL_Reg datetimepicker_functions[] =
{
	{ 0, 0 }
};

static int new_datetimepicker( lua_State* L )
{
	control_create( L, (uiControl*) uiNewDateTimePicker(), control_uiDateTimePicker, datetimepicker_functions );
	return 1;
}

static int new_timepicker( lua_State* L )
{
	control_create( L, (uiControl*) uiNewTimePicker(), control_uiDateTimePicker, datetimepicker_functions );
	return 1;
}

static int new_datepicker( lua_State* L )
{
	control_create( L, (uiControl*) uiNewDatePicker(), control_uiDateTimePicker, datetimepicker_functions );
	return 1;
}


DECLARE_GETTER( uiMultilineEntry, Text, string )
DECLARE_SETTER( uiMultilineEntry, SetText, string )
DECLARE_SETTER( uiMultilineEntry, Append, string )
DECLARE_GETTER( uiMultilineEntry, ReadOnly, boolean )
DECLARE_SETTER( uiMultilineEntry, SetReadOnly, boolean )
DECLARE_CALLBACK( uiMultilineEntry, OnChanged )

static luaL_Reg multilineentry_functions[] =
{
    { "Text", l_uiMultilineEntryText },
    { "SetText", l_uiMultilineEntrySetText },
    { "Append", l_uiMultilineEntryAppend },
    { "ReadOnly", l_uiMultilineEntryReadOnly },
    { "SetReadOnly", l_uiMultilineEntrySetReadOnly },
    { "OnChanged", l_uiMultilineEntryOnChanged },
	{ 0, 0 }
};

static int new_multilineentry( lua_State* L )
{
	control_create( L, (uiControl*) uiNewMultilineEntry(), control_uiMultilineEntry, multilineentry_functions );
	return 1;
}

static void callback_uiMenuItemOnClicked( uiMenuItem* i, uiWindow* w, void* d )
{
    invoke_callback( (lua_State*) d, i, callback_OnClicked );
}

DECLARE_ACTION( uiMenuItem, Enable )
DECLARE_ACTION( uiMenuItem, Disable )
DECLARE_GETTER( uiMenuItem, Checked, boolean )
DECLARE_SETTER( uiMenuItem, SetChecked, boolean )
DECLARE_CALLBACK_2( uiMenuItem, OnClicked )

static luaL_Reg menuitem_functions[] =
{
	{ "Enable", l_uiMenuItemEnable },
	{ "Disable", l_uiMenuItemDisable },
	{ "Checked", l_uiMenuItemChecked },
	{ "SetChecked", l_uiMenuItemSetChecked },
	{ "OnClicked", l_uiMenuItemOnClicked },
	{ 0, 0 }
};


int l_uiMenuAppendItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_uiMenu );
	if( m )
	{
		uiMenuItem* i = uiMenuAppendItem( m, luaL_checkstring( L, 2 ) );
		control_create( L, (uiControl*) i, control_uiMenuItem, menuitem_functions );
		return 1;
	}
	return 0;
}

int l_uiMenuAppendCheckItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_uiMenu );
	if( m )
	{
		uiMenuItem* i = uiMenuAppendCheckItem( m, luaL_checkstring( L, 2 ) );
		control_create( L, (uiControl*) i, control_uiMenuItem, menuitem_functions );
		return 1;
	}
	return 0;
}

int l_uiMenuAppendQuitItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_uiMenu );
	if( m )
	{
		uiMenuItem* i = uiMenuAppendQuitItem( m );
		control_create( L, (uiControl*) i, control_uiMenuItem, menuitem_functions );
		return 1;
	}
	return 0;
}

int l_uiMenuAppendPreferencesItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_uiMenu );
	if( m )
	{
		uiMenuItem* i = uiMenuAppendPreferencesItem( m );
		control_create( L, (uiControl*) i, control_uiMenuItem, menuitem_functions );
		return 1;
	}
	return 0;
}

int l_uiMenuAppendAboutItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_uiMenu );
	if( m )
	{
		uiMenuItem* i = uiMenuAppendAboutItem( m );
		control_create( L, (uiControl*) i, control_uiMenuItem, menuitem_functions );
		return 1;
	}
	return 0;
}

DECLARE_ACTION( uiMenu, AppendSeparator )

static luaL_Reg menu_functions[] =
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
	uiControl* c = (uiControl*) uiNewMenu( luaL_optlstring( L, 1, "Menu", NULL ) );
	control_create( L, c, control_uiMenu, menu_functions );
	return 1;
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

	control_create( L, (uiControl*) a, control_uiArea, area_functions );
	return 1;
}

static luaL_Reg fontbutton_functions[] =
{
	{ 0, 0 }
};

static int new_fontbutton( lua_State* L )
{
	control_create( L, (uiControl*) uiNewFontButton(), control_uiFontButton, fontbutton_functions );
	return 1;
}

static luaL_Reg colorbutton_functions[] =
{
	{ 0, 0 }
};

static int new_colorbutton( lua_State* L )
{
	control_create( L, (uiControl*) uiNewColorButton(), control_uiColorButton, colorbutton_functions );
	return 1;
}


static int openfile( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_uiWindow );
	if( w )
	{
		char* f = uiOpenFile( w );
		if( f )
		{
			lua_pushstring( L, f );
			uiFreeText( f );
			return 1;
		}
	}
	return 0;
}

static int savefile( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_uiWindow );
	if( w )
	{
		char* f = uiSaveFile( w );
		if( f )
		{
			lua_pushstring( L, f );
			uiFreeText( f );
			return 1;
		}
	}
	return 0;
}

static int msgbox( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_uiWindow );
	if( w )
	{
		uiMsgBox( w, luaL_checkstring( L, 2 ), luaL_checkstring( L, 3 ) );
	}
	return 0;
}

static int msgboxerror( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_uiWindow );
	if( w )
	{
		uiMsgBoxError( w, luaL_checkstring( L, 2 ), luaL_checkstring( L, 3 ) );
	}
	return 0;
}

luaL_Reg ui_functions[] =
{
	{ "Main", uimain },
	{ "Quit", uiquit },
	{ "QueueMain", queuemain },
	{ "ShouldQuit", onshouldquit },

	{ "NewWindow", new_window },
	{ "NewButton", new_button },
	{ "NewHBox", new_hbox },
	{ "NewVBox", new_vbox },
	{ "NewEntry", new_entry },
	{ "NewCheckbox", new_checkbox },
	{ "NewLabel", new_label },
	{ "NewTab", new_tab },
	{ "NewGroup", new_group },
	{ "NewSpinbox", new_spinbox },
	{ "NewProgressBar", new_progress },
	{ "NewSlider", new_slider },
	{ "NewHSeparator", new_hseparator },
	{ "NewCombobox", new_combobox },
	{ "NewRadioButtons", new_radiobuttons },
	{ "NewDateTimePicker", new_datetimepicker },
	{ "NewTimePicker", new_timepicker },
	{ "NewDatePicker", new_datepicker },
	{ "NewMultiLineEntry", new_multilineentry },
	{ "NewMenu", new_menu },
	{ "NewArea", new_area },
	{ "NewFontButton", new_fontbutton },
	{ "NewColorButton", new_colorbutton },
	{ "NewColourButton", new_colorbutton },

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

	open_platform_specific( L );

	return 1;
}
