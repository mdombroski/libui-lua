#include <lua.h>
#include <lauxlib.h>

// simplifies getters/setters
#define luaL_checkboolean( L, i ) lua_toboolean( L, i )

#include <ui.h>

// private header?
#include <common/controlsigs.h>

#ifndef MODULE_API
#define MODULE_API __attribute__((visibility("default")))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// TODO figure out how to free text in generic getters

// contained in platform specific file (libui-windows-lua.c or similar)
extern int open_platform_specific( lua_State* L );

static uiInitOptions init_options;


static int object_copy( lua_State* L, void* object );
static void* check_object( lua_State* L, int idx, int signature );


static int l_uiControlDestroy( lua_State* L );


enum
{
    // accept any control (except menu items)
    uiControlSignature = 0x5A5A5A5A,

    // menu, menu items
    uiMenuSignature = 0x54665434,
    uiMenuItemSignature = 0x295629A5,

	callback_ShouldQuit = 0xDA0,
	callback_OnClosing = 0xDA1,
	callback_OnClicked = 0xDA2,
	callback_OnToggled = 0xDA3,
	callback_OnChanged = 0xDA4,
	callback_OnSelected = 0xDA5,
};


static void register_callback( lua_State* L, int idx, void* sender, int callback )
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

static int invoke_callback( lua_State* L, void* id, int callback, int args )
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

#define DECLARE_CALLBACK_FUNCTION( typename, action ) \
    static void callback_ ## typename ## action( typename *c, void* data ) { \
        invoke_callback( (lua_State*) data, c, callback_ ## action, 0 ); \
    }
#define DECLARE_CALLBACK_REGISTER( typename, signature, action ) \
    static int l_ ## typename ## action( lua_State* L ) { \
        typename* c = (typename*) check_object( L, 1, signature ); \
        register_callback( L, 2, c, callback_ ## action ); \
        if( lua_type( L, 2 ) == LUA_TFUNCTION ) \
            typename ## action( c, callback_ ## typename ## action, L ); \
        lua_pushvalue( L, 1 ); \
        return 1; \
    }
#define DECLARE_CALLBACK( typename, action ) \
    DECLARE_CALLBACK_FUNCTION( typename, action ) \
    DECLARE_CALLBACK_REGISTER( typename, typename ## Signature, action )

#define DECLARE_SETTER( typename, action, type ) \
    int l_ ## typename ## action( lua_State* L ) { \
        typename ## action( (typename*) check_object( L, 1, typename ## Signature ), luaL_check ## type ( L, 2 ) ); \
        lua_pushvalue( L, 1 ); \
        return 1; \
    }

#define DECLARE_GETTER( typename, action, type ) \
    int l_ ## typename ## action( lua_State* L ) { \
        lua_push ## type ( L, typename ## action ( (typename*) check_object( L, 1, typename ## Signature ) ) ); \
        return 1; \
    }

#define DECLARE_ACTION( typename, action ) \
    int l_ ## typename ## action( lua_State* L ) { \
        typename ## action ( (typename*) check_object( L, 1, typename ## Signature ) ); \
        lua_pushvalue( L, 1 ); \
        return 1; \
    }

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
	{
		uiOnShouldQuit( callback_uiShouldQuit, L );
	}
	else
	{
		uiOnShouldQuit( NULL, NULL );
	}

	return 0;
}


static int object_gc( lua_State* L )
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
static void object_create( lua_State* L, void* object, int signature, ... )
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
}

static int object_copy( lua_State* L, void* object )
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

static void* check_object( lua_State* L, int idx, int signature )
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


static int l_uiControlDestroy( lua_State* L )
{
	uiControl* c = check_object( L, 1, uiControlSignature );
    uiControlDestroy( c );

    // destroy registry meta table
    lua_pushlightuserdata( L, c );
    lua_pushnil( L );
    lua_settable( L, LUA_REGISTRYINDEX );

	return 0;
}

DECLARE_GETTER( uiControl, Handle, integer )

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

DECLARE_GETTER( uiControl, Visible, boolean )
DECLARE_ACTION( uiControl, Show )
DECLARE_ACTION( uiControl, Hide )
DECLARE_GETTER( uiControl, Enabled, boolean )
DECLARE_ACTION( uiControl, Enable )
DECLARE_ACTION( uiControl, Disable )
DECLARE_GETTER( uiControl, Toplevel, boolean )
DECLARE_GETTER( uiControl, EnabledToUser, boolean )

static int control_free( lua_State* L )
{
	printf( "STUB uiFreeControl\n" );
	return 0;
}

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
	{ "Free", control_free },
	{ "EnabledToUser", l_uiControlEnabledToUser },
	{ 0, 0 }
};

static int callback_uiWindowOnClosing( uiWindow* w, void* d )
{
    invoke_callback( (lua_State*) d, w, callback_OnClosing, 0 );
    return 0;
}

int l_uiWindowTitle( lua_State* L )
{
    char* s = uiWindowTitle( (uiWindow*) check_object( L, 1, uiWindowSignature ) );
    lua_pushstring( L, s );
    uiFreeText( s );
    return 1;
}
//DECLARE_GETTER( uiWindow, Title, string )
DECLARE_SETTER( uiWindow, SetTitle, string )
DECLARE_GETTER( uiWindow, Margined, boolean)
DECLARE_SETTER( uiWindow, SetMargined, boolean )
DECLARE_CALLBACK_REGISTER( uiWindow, uiWindowSignature, OnClosing )

static int window_setchild( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_object( L, 1, uiWindowSignature );
	uiControl* c = (uiControl*) check_object( L, 2, uiControlSignature );
	uiWindowSetChild( w, c );
	lua_pushvalue( L, 1 );
	return 1;
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
	object_create( L, w, uiWindowSignature, control_common, window_functions, 0 );

	return 1;
}


int l_uiButtonText( lua_State* L )
{
    char* s = uiButtonText( (uiButton*) check_object( L, 1, uiButtonSignature ) );
    lua_pushstring( L, s );
    return 1;
}
//DECLARE_GETTER( uiButton, Text, string )
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
	object_create( L, b, uiButtonSignature, control_common, button_functions, 0 );

	return 1;
}


static int box_append( lua_State* L )
{
	uiBox* b = (uiBox*) check_object( L, 1, uiBoxSignature );
	uiControl* o = (uiControl*) check_object( L, 2, uiControlSignature );
	uiBoxAppend( b, o, lua_toboolean( L, 3 ) );
	lua_pushvalue( L, 1 );
	return 1;
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
	object_create( L, uiNewHorizontalBox(), uiBoxSignature, control_common, box_functions, 0 );
	return 1;
}

static int new_vbox( lua_State* L )
{
	object_create( L, uiNewVerticalBox(), uiBoxSignature, control_common, box_functions, 0 );
	return 1;
}


static int l_uiCheckboxText( lua_State* L )
{
    char* s = uiCheckboxText( (uiCheckbox*) check_object( L, 1, uiCheckboxSignature ) );
    lua_pushstring( L, s );
    uiFreeText( s );
    return 1;
}

static void callback_uiCheckboxOnToggled( uiCheckbox* c, void* d )
{
    lua_State* L = (lua_State*) d;
    if( L )
    {
        lua_pushboolean( L, uiCheckboxChecked( c ) );
        invoke_callback( L, c, callback_OnToggled, 1 );
    }
}

//DECLARE_GETTER( uiCheckbox, Text, string )
DECLARE_SETTER( uiCheckbox, SetText, string )
DECLARE_GETTER( uiCheckbox, Checked, boolean )
DECLARE_SETTER( uiCheckbox, SetChecked, boolean )
DECLARE_CALLBACK_REGISTER( uiCheckbox, uiCheckboxSignature, OnToggled )

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
    uiCheckbox* c = uiNewCheckbox( luaL_optlstring( L, 1, "Checkbox", NULL ) );
	object_create( L, c, uiCheckboxSignature, control_common, checkbox_functions, 0 );
	return 1;
}


int l_uiEntryText( lua_State* L )
{
    char* s = uiEntryText( (uiEntry*) check_object( L, 1, uiEntrySignature ) );
    lua_pushstring( L, s );
    uiFreeText( s );
    return 1;
}

static void callback_uiEntryOnChanged( uiEntry* c, void* d )
{
    lua_State* L = (lua_State*) d;
    if( L )
    {
        char* str = uiEntryText( c );
        lua_pushstring( L, str );
        uiFreeText( str );
        invoke_callback( L, c, callback_OnChanged, 1 );
    }
}

//DECLARE_GETTER( uiEntry, Text, string )
DECLARE_SETTER( uiEntry, SetText, string )
DECLARE_GETTER( uiEntry, ReadOnly, boolean )
DECLARE_SETTER( uiEntry, SetReadOnly, boolean )
DECLARE_CALLBACK_REGISTER( uiEntry, uiEntrySignature, OnChanged )

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
	object_create( L, uiNewEntry(), uiEntrySignature, control_common, entry_functions, 0 );
	return 1;
}


int l_uiLabelText( lua_State* L )
{
    char* s = uiLabelText( (uiLabel*) check_object( L, 1, uiLabelSignature ) );
    lua_pushstring( L, s );
    uiFreeText( s );
    return 1;
}
//DECLARE_GETTER( uiLabel, Text, string )
DECLARE_SETTER( uiLabel, SetText, string )

luaL_Reg label_functions[] =
{
	{ "Text", l_uiLabelText },
	{ "SetText", l_uiLabelSetText },
	{ 0, 0 }
};

static int new_label( lua_State* L )
{
    uiLabel* l = uiNewLabel( luaL_optlstring( L, 1, "Label", NULL ) );
	object_create( L, l, uiLabelSignature, control_common, label_functions, 0 );
	return 1;
}


static int tab_append( lua_State* L )
{
	uiTab* t = (uiTab*) check_object( L, 1, uiTabSignature );
	uiControl* c = check_object( L, 3, uiControlSignature );
    uiTabAppend( t, luaL_checkstring( L, 2 ), c );
    lua_pushvalue( L, 1 );
    return 1;
}

static int tab_insert( lua_State* L )
{
	uiTab* t = (uiTab*) check_object( L, 1, uiTabSignature );
	uiControl* c = check_object( L, 4, uiControlSignature );
    uiTabInsertAt( t, luaL_checkstring( L, 3 ), luaL_checkinteger( L, 2 ), c );
	lua_pushvalue( L, 1 );
    return 1;
}

DECLARE_SETTER( uiTab, Delete, integer )
DECLARE_GETTER( uiTab, NumPages, integer )

static int l_uiTabMargined( lua_State* L )
{
	uiTab* t = (uiTab*) check_object( L, 1, uiTabSignature );
    lua_pushboolean( L, uiTabMargined( t, luaL_checkinteger( L, 2 ) ) );
	return 0;
}

static int l_uiTabSetMargined( lua_State* L )
{
	uiTab* t = (uiTab*) check_object( L, 1, uiTabSignature );
    uiTabSetMargined( t, luaL_checkinteger( L, 2 ), lua_toboolean( L, 3 ) );
	lua_pushvalue( L, 1 );
	return 1;
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
	object_create( L, uiNewTab(), uiTabSignature, control_common, tab_functions, 0 );
	return 1;
}


int l_uiGroupTitle( lua_State* L )
{
    char* s = uiGroupTitle( (uiGroup*) check_object( L, 1, uiGroupSignature ) );
    lua_pushstring( L, s );
    uiFreeText( s );
    return 1;
}
//DECLARE_GETTER( uiGroup, Title, string )
DECLARE_SETTER( uiGroup, SetTitle, string )
DECLARE_GETTER( uiGroup, Margined, boolean )
DECLARE_SETTER( uiGroup, SetMargined, boolean )

static int group_setchild( lua_State* L )
{
	uiGroup* g = (uiGroup*) check_object( L, 1, uiGroupSignature );
	uiControl* c = check_object( L, 2, uiControlSignature );
	uiGroupSetChild( g, c );
	lua_pushvalue( L, 1 );
	return 1;
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
    uiGroup* g = uiNewGroup( luaL_optlstring( L, 1, "Label", NULL ) );
	object_create( L, g, uiGroupSignature, control_common, group_functions, 0 );
	return 1;
}

static void callback_uiSpinboxOnChanged( uiSpinbox* c, void* d )
{
    lua_State* L = (lua_State*) d;
    if( L )
    {
        lua_pushinteger( L, uiSpinboxValue( c ) );
        invoke_callback( L, c, callback_OnChanged, 1 );
    }
}

DECLARE_GETTER( uiSpinbox, Value, integer )
DECLARE_SETTER( uiSpinbox, SetValue, integer )
DECLARE_CALLBACK_REGISTER( uiSpinbox, uiSpinboxSignature, OnChanged )

static luaL_Reg spinbox_functions[] =
{
	{ "Value", l_uiSpinboxValue },
	{ "SetValue", l_uiSpinboxSetValue },
	{ "OnChanged", l_uiSpinboxOnChanged },
	{ 0, 0 }
};

static int new_spinbox( lua_State* L )
{
    uiSpinbox* s = uiNewSpinbox( luaL_optinteger( L, 1, 0 ), luaL_optinteger( L, 2, 100 ) );
	object_create( L, s, uiSpinboxSignature, control_common, spinbox_functions, 0 );
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
	object_create( L, uiNewProgressBar(), uiProgressBarSignature, control_common, progress_functions, 0 );
	return 1;
}


static void callback_uiSliderOnChanged( uiSlider* c, void* d )
{
    lua_State* L = (lua_State*) d;
    if( L )
    {
        lua_pushinteger( L, uiSliderValue( c ) );
        invoke_callback( L, c, callback_OnChanged, 1 );
    }
}

DECLARE_GETTER( uiSlider, Value, integer )
DECLARE_SETTER( uiSlider, SetValue, integer )
DECLARE_CALLBACK_REGISTER( uiSlider, uiSliderSignature, OnChanged )

static luaL_Reg slider_functions[] =
{
	{ "Value", l_uiSliderValue },
	{ "SetValue", l_uiSliderSetValue },
	{ "OnChanged", l_uiSliderOnChanged },
	{ 0, 0 }
};

static int new_slider( lua_State* L )
{
    uiSlider* s = uiNewSlider( luaL_optinteger( L, 1, 0 ), luaL_optinteger( L, 2, 100 ) );
	object_create( L, s, uiSliderSignature, control_common, slider_functions, 0 );
	return 1;
}


static int new_hseparator( lua_State* L )
{
	object_create( L, uiNewHorizontalSeparator(), uiSeparatorSignature, control_common, 0 );
	return 1;
}

static void callback_uiComboboxOnSelected( uiCombobox* c, void* d )
{
    lua_State* L = (lua_State*) d;
    if( L )
    {
        lua_pushinteger( L, uiComboboxSelected( c ) );
        invoke_callback( L, c, callback_OnSelected, 1 );
    }
}

DECLARE_SETTER( uiCombobox, Append, string )
DECLARE_GETTER( uiCombobox, Selected, integer )
DECLARE_SETTER( uiCombobox, SetSelected, integer )
DECLARE_CALLBACK_REGISTER( uiCombobox, uiComboboxSignature, OnSelected )

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
		object_create( L, uiNewEditableCombobox(), uiComboboxSignature, control_common, combobox_functions, 0 );
	}
	else
	{
		object_create( L, uiNewCombobox(), uiComboboxSignature, control_common, combobox_functions, 0 );
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
	object_create( L, uiNewRadioButtons(), uiRadioButtonsSignature, control_common, radiobuttons_functions, 0 );
	return 1;
}


static luaL_Reg datetimepicker_functions[] =
{
	{ 0, 0 }
};

static int new_datetimepicker( lua_State* L )
{
	object_create( L, uiNewDateTimePicker(), uiDateTimePickerSignature, control_common, datetimepicker_functions, 0 );
	return 1;
}

static int new_timepicker( lua_State* L )
{
	object_create( L, uiNewTimePicker(), uiDateTimePickerSignature, control_common, datetimepicker_functions, 0 );
	return 1;
}

static int new_datepicker( lua_State* L )
{
	object_create( L, uiNewDatePicker(), uiDateTimePickerSignature, control_common, datetimepicker_functions, 0 );
	return 1;
}


int l_uiMultilineEntryText( lua_State* L )
{
    char* s = uiMultilineEntryText( (uiMultilineEntry*) check_object( L, 1, uiMultilineEntrySignature ) );
    lua_pushstring( L, s );
    uiFreeText( s );
    return 1;
}

static void callback_uiMultilineEntryOnChanged( uiMultilineEntry* c, void* d )
{
    lua_State* L = (lua_State*) d;
    if( L )
    {
        char* str = uiMultilineEntryText( c );
        lua_pushstring( L, str );
        uiFreeText( str );
        invoke_callback( L, c, callback_OnChanged, 1 );
    }
}

//DECLARE_GETTER( uiMultilineEntry, Text, string )
DECLARE_SETTER( uiMultilineEntry, SetText, string )
DECLARE_SETTER( uiMultilineEntry, Append, string )
DECLARE_GETTER( uiMultilineEntry, ReadOnly, boolean )
DECLARE_SETTER( uiMultilineEntry, SetReadOnly, boolean )
DECLARE_CALLBACK_REGISTER( uiMultilineEntry, uiMultilineEntrySignature, OnChanged )

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
	object_create( L, uiNewMultilineEntry(), uiMultilineEntrySignature, control_common, multilineentry_functions, 0 );
	return 1;
}

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


int l_uiMenuAppendItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_object( L, 1, uiMenuSignature );
	uiMenuItem* i = uiMenuAppendItem( m, luaL_checkstring( L, 2 ) );
    object_create( L, i, uiMenuItemSignature, menuitem_functions, 0 );
    return 1;
}

int l_uiMenuAppendCheckItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_object( L, 1, uiMenuSignature );
    uiMenuItem* i = uiMenuAppendCheckItem( m, luaL_checkstring( L, 2 ) );
    object_create( L, i, uiMenuItemSignature, menuitem_functions, 0 );
    return 1;
}

int l_uiMenuAppendQuitItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_object( L, 1, uiMenuSignature );
    uiMenuItem* i = uiMenuAppendQuitItem( m );
    object_create( L, i, uiMenuItemSignature, menuitem_functions, 0 );
    return 1;
}

int l_uiMenuAppendPreferencesItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_object( L, 1, uiMenuSignature );
    uiMenuItem* i = uiMenuAppendPreferencesItem( m );
    object_create( L, i, uiMenuItemSignature, menuitem_functions, 0 );
    return 1;
}

int l_uiMenuAppendAboutItem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_object( L, 1, uiMenuSignature );
    uiMenuItem* i = uiMenuAppendAboutItem( m );
    object_create( L, i, uiMenuItemSignature, menuitem_functions, 0 );
    return 1;
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
	void* m = uiNewMenu( luaL_optlstring( L, 1, "Menu", NULL ) );
	object_create( L, m, uiMenuSignature, menu_functions, 0 );
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

	object_create( L, a, uiAreaSignature, control_common, area_functions, 0 );
	return 1;
}

static luaL_Reg fontbutton_functions[] =
{
	{ 0, 0 }
};

static int new_fontbutton( lua_State* L )
{
	object_create( L, uiNewFontButton(), uiFontButtonSignature, control_common, fontbutton_functions, 0 );
	return 1;
}


static int l_uiColorButtonColor( lua_State* L )
{
    double r,g,b,a;
    uiColorButtonColor( (uiColorButton*) check_object( L, 1, uiColorButtonSignature ), &r, &g, &b, &a );
    lua_pushnumber( L, r );
    lua_pushnumber( L, g );
    lua_pushnumber( L, b );
    lua_pushnumber( L, a );
    return 4;
}

static int l_uiColorButtonSetColor( lua_State* L )
{
    uiColorButtonSetColor( (uiColorButton*) check_object( L, 1, uiColorButtonSignature ),
                          luaL_checknumber( L, 2 ),     // r
                          luaL_checknumber( L, 3 ),     // g
                          luaL_checknumber( L, 4 ),     // b
                          luaL_checknumber( L, 5 ) );   // a
    lua_pushvalue( L, 1 );
    return 1;
}

static void callback_uiColorButtonOnChanged( uiColorButton* c, void* d )
{
    lua_State* L = (lua_State*) d;
    if( L )
    {
        double r,g,b,a;
        uiColorButtonColor( c, &r, &g, &b, &a );
        lua_pushnumber( L, r );
        lua_pushnumber( L, g );
        lua_pushnumber( L, b );
        lua_pushnumber( L, a );
        invoke_callback( L, c, callback_OnChanged, 4 );
    }
}

DECLARE_CALLBACK_REGISTER( uiColorButton, uiColorButtonSignature, OnChanged )

static luaL_Reg colorbutton_functions[] =
{
    { "Color", l_uiColorButtonColor },
    { "Colour", l_uiColorButtonColor },
    { "SetColor", l_uiColorButtonSetColor },
    { "SetColour", l_uiColorButtonSetColor },
    { "OnChanged", l_uiColorButtonOnChanged },
	{ 0, 0 }
};

static int new_colorbutton( lua_State* L )
{
	object_create( L, uiNewColorButton(), uiColorButtonSignature, control_common, colorbutton_functions, 0 );
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
	{ "NewMultilineEntry", new_multilineentry },
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
