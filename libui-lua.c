#include <lua.h>
#include <lauxlib.h>

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
	control_window = 0xAA1,
	control_button = 0xAA2,
	control_box = 0xAA3,
	control_checkbox = 0xAA4,
	control_entry = 0xAA5,
	control_label = 0xAA6,
	control_tab = 0xAA7,
	control_group = 0xAA8,
	control_spinbox = 0xAA9,
	control_progressbar = 0xAAA,
	control_slider = 0xAAB,
	control_separator = 0xAAC,
	control_combobox = 0xAAD,
	control_radiobuttons = 0xAAE,
	control_datetimepicker = 0xAAF,
	control_multilineentry = 0xAB0,
	control_menu = 0xAB1,
	control_menuitem = 0xAB2,
	control_area = 0xAB3,
	control_fontbutton = 0xAB4,
	control_colorbutton = 0xAB5,

	callback_shouldquit = 0xDA0,
	callback_clicked = 0xDA1,
	callback_onclosing = 0xDA2,
	callback_toggled = 0xDA3,
	callback_changed = 0xDA4,
	callback_selected = 0xDA5,
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

static int control_handle( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	if( c )
	{
		lua_pushinteger( L, uiControlHandle( c ) );
		return 1;
	}
	return 0;
}

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

static int control_visible( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	if( c )
	{
		lua_pushboolean( L, uiControlVisible( c ) );
		return 1;
	}

	return 0;
}

static int control_show( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	if( c )
	{
		uiControlShow( c );
	}

	return 0;
}

static int control_hide( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	if( c )
	{
		uiControlHide( c );
	}

	return 0;
}

static int control_enabled( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	if( c )
	{
		lua_pushboolean( L, uiControlEnabled( c ) );
		return 1;
	}

	return 0;
}

static int control_enable( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	if( c )
	{
		uiControlEnable( c );
	}

	return 0;
}

static int control_disable( lua_State* L )
{
	uiControl* c = check_control( L, -1, 0 );
	if( c )
	{
		uiControlDisable( c );
	}

	return 0;
}

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
	{ "Handle", control_handle },
	{ "Parent", control_parent },
	{ "SetParent", control_setparent },
	{ "TopLevel", control_toplevel },
	{ "Visible", control_visible },
	{ "Show", control_show },
	{ "Hide", control_hide },
	{ "Enabled", control_enabled },
	{ "Enable", control_enable },
	{ "Disable", control_disable },
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


static int window_title( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_window );
	if( w )
	{
		lua_pushstring( L, uiWindowTitle( w ) );
		return 1;
	}
	return 0;
}

static int window_settitle( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_window );
	if( w )
	{
		uiWindowSetTitle( w, luaL_checkstring( L, 2 ) );
	}
	return 0;
}

static int window_onclosing_callback( uiWindow* c, void* data )
{
	invoke_callback( (lua_State*) data, c, callback_onclosing );
	return 0;
}

static int window_onclosing( lua_State* L )
{
	// TODO
	uiWindow* w = (uiWindow*) check_control( L, 1, control_window );
	if( w )
	{
		register_callback( L, 2, w, callback_onclosing );

		if( lua_type( L, 2 ) == LUA_TFUNCTION )
		{
			uiWindowOnClosing( w, window_onclosing_callback, L );
		}
		else
		{
			uiWindowOnClosing( w, NULL, NULL );
		}
	}
	return 0;
}

static int window_setchild( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_window );
	uiControl* c = (uiControl*) check_control( L, 2, 0 );
	if( w && c && ( uiControl(w) != c ) )
	{
		uiWindowSetChild( w, c );
	}
	return 0;
}

static int window_margined( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_window );
	if( w )
	{
		lua_pushboolean( L, uiWindowMargined( w ) );
		return 1;
	}
	return 0;
}

static int window_setmargined( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_window );
	if( w )
	{
		uiWindowSetMargined( w, lua_toboolean( L, 2 ) );
	}
	return 0;
}

static luaL_Reg window_functions[] =
{
	{ "Title", window_title },
	{ "SetTitle", window_settitle },
	{ "Margined", window_margined },
	{ "SetMargined", window_setmargined },
	{ "SetChild", window_setchild },
	{ "OnClosing", window_onclosing },
	{ 0, 0 }
};

static int new_window( lua_State* L )
{
	uiWindow* w = uiNewWindow( luaL_optlstring( L, 1, "Lua", NULL ), luaL_optinteger( L, 2, 640 ), luaL_optinteger( L, 3, 480 ), lua_toboolean( L, 4 ) );
	control_create( L, (uiControl*) w, control_window, window_functions );

	return 1;
}


static int button_text( lua_State* L )
{
	uiButton* b = (uiButton*) check_control( L, 1, control_button );
	if( b )
	{
		lua_pushstring( L, uiButtonText( b ) );
		return 1;
	}
	return 0;
}

static int button_settext( lua_State* L )
{
	uiButton* b = (uiButton*) check_control( L, 1, control_button );
	if( b )
	{
		uiButtonSetText( b, luaL_checkstring( L, 2 ) );
	}
	return 0;
}

static void button_clicked_callback( uiButton* c, void* data )
{
	invoke_callback( (lua_State*) data, c, callback_clicked );
}

static int button_onclicked( lua_State* L )
{
	uiButton* b = (uiButton*) check_control( L, 1, control_button );
	if( b )
	{
		register_callback( L, 2, b, callback_clicked );

		if( lua_type( L, 2 ) == LUA_TFUNCTION )
		{
			uiButtonOnClicked( b, button_clicked_callback, L );
		}
		else
		{
			uiButtonOnClicked( b, NULL, NULL );
		}
	}
	return 0;
}

static luaL_Reg button_functions[] =
{
	{ "Text", button_text },
	{ "SetText", button_settext },
	{ "OnClicked", button_onclicked },
	{ 0, 0 }
};

static int new_button( lua_State* L )
{
	uiButton* b = uiNewButton( luaL_optlstring( L, 1, "Button", NULL ) );
	control_create( L, (uiControl*) b, control_button, button_functions );

	return 1;
}


static int box_append( lua_State* L )
{
	uiBox* b = (uiBox*) check_control( L, 1, control_box );
	uiControl* o = (uiControl*) check_control( L, 2, 0 );
	if( b && o )
	{
		uiBoxAppend( b, o, lua_toboolean( L, 3 ) );
	}
	return 0;
}

static int box_delete( lua_State* L )
{
	uiBox* b = (uiBox*) check_control( L, 1, control_box );
	if( b )
	{
		uiBoxDelete( b, luaL_checkinteger( L, 2 ) );
	}
	return 0;
}

static int box_padded( lua_State* L )
{
	uiBox* b = (uiBox*) check_control( L, 1, control_box );
	if( b )
	{
		lua_pushboolean( L, uiBoxPadded( b ) );
		return 1;
	}
	return 0;
}

static int box_setpadded( lua_State* L )
{
	uiBox* b = (uiBox*) check_control( L, 1, control_box );
	if( b )
	{
		uiBoxSetPadded( b, lua_toboolean( L, 2 ) );
	}
	return 0;
}

luaL_Reg box_functions[] =
{
	{ "Append", box_append },
	{ "Delete", box_delete },
	{ "Padded", box_padded },
	{ "SetPadded", box_setpadded },
	{ 0, 0 }
};

static int new_hbox( lua_State* L )
{
	control_create( L, (uiControl*) uiNewHorizontalBox(), control_box, box_functions );
	return 1;
}

static int new_vbox( lua_State* L )
{
	control_create( L, (uiControl*) uiNewVerticalBox(), control_box, box_functions );
	return 1;
}


static int checkbox_text( lua_State* L )
{
	uiCheckbox* c = (uiCheckbox*) check_control( L, 1, control_checkbox );
	if( c )
	{
		lua_pushstring( L, uiCheckboxText( c ) );
		return 1;
	}
	return 0;
}

static int checkbox_settext( lua_State* L )
{
	uiCheckbox* c = (uiCheckbox*) check_control( L, 1, control_checkbox );
	if( c )
	{
		uiCheckboxSetText( c, luaL_checkstring( L, 2 ) );
	}
	return 0;
}

static void checkbox_toggled_callback( uiCheckbox* c, void* d )
{
	invoke_callback( (lua_State*) d, c, callback_toggled );
}

static int checkbox_ontoggled( lua_State* L )
{
	uiCheckbox* c = (uiCheckbox*) check_control( L, 1, control_checkbox );
	if( c )
	{
		register_callback( L, 2, c, callback_toggled );

		if( lua_type( L, 2 ) == LUA_TFUNCTION )
		{
			uiCheckboxOnToggled( c, checkbox_toggled_callback, L );
		}
		else
		{
			uiCheckboxOnToggled( c, NULL, NULL );
		}
	}
	return 0;
}

static int checkbox_checked( lua_State* L )
{
	uiCheckbox* c = (uiCheckbox*) check_control( L, 1, control_checkbox );
	if( c )
	{
		lua_pushboolean( L, uiCheckboxChecked( c ) );
		return 1;
	}
	return 0;
}

static int checkbox_setchecked( lua_State* L )
{
	uiCheckbox* c = (uiCheckbox*) check_control( L, 1, control_checkbox );
	if( c )
	{
		uiCheckboxSetChecked( c, lua_toboolean( L, 2 ) );
	}
	return 0;
}

luaL_Reg checkbox_functions[] =
{
	{ "Text", checkbox_text },
	{ "SetText", checkbox_settext },
	{ "OnToggled", checkbox_ontoggled },
	{ "Checked", checkbox_checked },
	{ "SetChecked", checkbox_setchecked },
	{ 0, 0 }
};

static int new_checkbox( lua_State* L )
{
	control_create( L, (uiControl*) uiNewCheckbox( luaL_optlstring( L, 1, "Checkbox", NULL ) ), control_checkbox, checkbox_functions );
	return 1;
}


static int entry_text( lua_State* L )
{
	uiEntry* c = (uiEntry*) check_control( L, 1, control_entry );
	if( c )
	{
		lua_pushstring( L, uiEntryText( c ) );
		return 1;
	}
	return 0;
}

static int entry_settext( lua_State* L )
{
	uiEntry* c = (uiEntry*) check_control( L, 1, control_entry );
	if( c )
	{
		uiEntrySetText( c, luaL_checkstring( L, 2 ) );
	}
	return 0;
}

static void entry_onchanged_callback( uiEntry* e, void* d )
{
	invoke_callback( (lua_State*) d, e, callback_changed );
}

static int entry_onchanged( lua_State* L )
{
	uiEntry* c = (uiEntry*) check_control( L, 1, control_entry );
	if( c )
	{
		register_callback( L, 2, c, callback_changed );

		if( lua_type( L, 2 ) == LUA_TFUNCTION )
		{
			uiEntryOnChanged( c, entry_onchanged_callback, L );
		}
		else
		{
			uiEntryOnChanged( c, NULL, NULL );
		}
	}
	return 0;
}

static int entry_readonly( lua_State* L )
{
	uiEntry* c = (uiEntry*) check_control( L, 1, control_entry );
	if( c )
	{
		lua_pushboolean( L, uiEntryReadOnly( c ) );
		return 1;
	}
	return 0;
}

static int entry_setreadonly( lua_State* L )
{
	uiEntry* c = (uiEntry*) check_control( L, 1, control_entry );
	if( c )
	{
		uiEntrySetReadOnly( c, lua_toboolean( L, 2 ) );
	}
	return 0;
}

luaL_Reg entry_functions[] =
{
	{ "Text", entry_text },
	{ "SetText", entry_settext },
	{ "OnChanged", entry_onchanged },
	{ "ReadOnly", entry_readonly },
	{ "SetReadOnly", entry_setreadonly },
	{ 0, 0 }
};

static int new_entry( lua_State* L )
{
	control_create( L, (uiControl*) uiNewEntry(), control_entry, entry_functions );
	return 1;
}

static int label_text( lua_State* L )
{
	uiLabel* c = (uiLabel*) check_control( L, 1, control_label );
	if( c )
	{
		lua_pushstring( L, uiLabelText( c ) );
		return 1;
	}
	return 0;
}

static int label_settext( lua_State* L )
{
	uiLabel* c = (uiLabel*) check_control( L, 1, control_label );
	if( c )
	{
		uiLabelSetText( c, luaL_checkstring( L, 2 ) );
	}
	return 0;
}

luaL_Reg label_functions[] =
{
	{ "Text", label_text },
	{ "SetText", label_settext },
	{ 0, 0 }
};

static int new_label( lua_State* L )
{
	control_create( L, (uiControl*) uiNewLabel( luaL_optlstring( L, 1, "Label", NULL ) ), control_label, label_functions );
	return 1;
}


static int tab_append( lua_State* L )
{
	uiTab* t = (uiTab*) check_control( L, 1, control_tab );
	uiControl* c = check_control( L, 3, 0 );

	if( t && c )
	{
		uiTabAppend( t, luaL_checkstring( L, 2 ), c );
	}

	return 0;
}

static int tab_insert( lua_State* L )
{
	uiTab* t = (uiTab*) check_control( L, 1, control_tab );
	uiControl* c = check_control( L, 4, 0 );

	if( t && c )
	{
		uiTabInsertAt( t, luaL_checkstring( L, 3 ), luaL_checkinteger( L, 2 ), c );
	}

	return 0;
}

static int tab_delete( lua_State* L )
{
	uiTab* t = (uiTab*) check_control( L, 1, control_tab );
	if( t )
	{
		uiTabDelete( t, luaL_checkinteger( L, 2 ) );
	}
	return 0;
}

static int tab_numpages( lua_State* L )
{
	uiTab* t = (uiTab*) check_control( L, 1, control_tab );
	if( t )
	{
		lua_pushinteger( L, uiTabNumPages( t ) );
		return 1;
	}
	return 0;
}

static int tab_margined( lua_State* L )
{
	uiTab* t = (uiTab*) check_control( L, 1, control_tab );
	if( t )
	{
		lua_pushboolean( L, uiTabMargined( t, luaL_checkinteger( L, 2 ) ) );
	}
	return 0;
}

static int tab_setmargined( lua_State* L )
{
	uiTab* t = (uiTab*) check_control( L, 1, control_tab );
	if( t )
	{
		uiTabSetMargined( t, luaL_checkinteger( L, 2 ), lua_toboolean( L, 3 ) );
	}
	return 0;
}

static luaL_Reg tab_functions[] =
{
	{ "Append", tab_append },
	{ "Insert", tab_insert },
	{ "Delete", tab_delete },
	{ "NumPages", tab_numpages },
	{ "Margined", tab_margined },
	{ "SetMargined", tab_setmargined },
	{ 0, 0 }
};

static int new_tab( lua_State* L )
{
	control_create( L, (uiControl*) uiNewTab(), control_tab, tab_functions );
	return 1;
}


static int group_title( lua_State* L )
{
	uiGroup* g = (uiGroup*) check_control( L, 1, control_group );
	if( g )
	{
		lua_pushstring( L, uiGroupTitle( g ) );
		return 1;
	}
	return 0;
}

static int group_settitle( lua_State* L )
{
	uiGroup* g = (uiGroup*) check_control( L, 1, control_group );
	if( g )
	{
		uiGroupSetTitle( g, luaL_checkstring( L, 2 ) );
	}
	return 0;
}

static int group_setchild( lua_State* L )
{
	uiGroup* g = (uiGroup*) check_control( L, 1, control_group );
	uiControl* c = check_control( L, 2, 0 );
	if( g && c )
	{
		uiGroupSetChild( g, c );
	}
	return 0;
}

static int group_margined( lua_State* L )
{
	uiGroup* g = (uiGroup*) check_control( L, 1, control_group );
	if( g )
	{
		lua_pushboolean( L, uiGroupMargined( g ) );
		return 1;
	}
	return 0;
}

static int group_setmargined( lua_State* L )
{
	uiGroup* g = (uiGroup*) check_control( L, 1, control_group );
	if( g )
	{
		uiGroupSetMargined( g, lua_toboolean( L, 2 ) );
	}
	return 0;
}

static luaL_Reg group_functions[] =
{
	{ "Title", group_title },
	{ "SetTitle", group_settitle },
	{ "SetChild", group_setchild },
	{ "Margined", group_margined },
	{ "SetMargined", group_setmargined },
	{ 0, 0 }
};

static int new_group( lua_State* L )
{
	control_create( L, (uiControl*) uiNewGroup( luaL_optlstring( L, 1, "Label", NULL ) ), control_group, group_functions );
	return 1;
}


static int spinbox_value( lua_State* L )
{
	uiSpinbox* s = (uiSpinbox*) check_control( L, 1, control_spinbox );
	if( s )
	{
		lua_pushinteger( L, uiSpinboxValue( s ) );
		return 1;
	}
	return 0;
}

static int spinbox_setvalue( lua_State* L )
{
	uiSpinbox* s = (uiSpinbox*) check_control( L, 1, control_spinbox );
	if( s )
	{
		uiSpinboxSetValue( s, luaL_checkinteger( L, 2 ) );
	}
	return 0;
}

static void spinbox_changed_callback( uiSpinbox* s, void* d )
{
	printf( "spinbox change\n" );
	invoke_callback( (lua_State*) d, s, callback_changed );
}

static int spinbox_onchanged( lua_State* L )
{
	uiSpinbox* s = (uiSpinbox*) check_control( L, 1, control_spinbox );
	if( s )
	{
		register_callback( L, 2, s, callback_changed );
		if( lua_isfunction( L, 2 ) )
			uiSpinboxOnChanged( s, spinbox_changed_callback, L );
		else
			uiSpinboxOnChanged( s, NULL, NULL );
	}
	return 0;
}

static luaL_Reg spinbox_functions[] =
{
	{ "Value", spinbox_value },
	{ "SetValue", spinbox_setvalue },
	{ "OnChanged", spinbox_onchanged },
	{ 0, 0 }
};

static int new_spinbox( lua_State* L )
{
	control_create( L, (uiControl*) uiNewSpinbox( luaL_optinteger( L, 1, 0 ), luaL_optinteger( L, 2, 100 ) ), control_spinbox, spinbox_functions );
	return 1;
}


static int progress_setvalue( lua_State* L )
{
	uiProgressBar* b = (uiProgressBar*) check_control( L, 1, control_progressbar );
	if( b )
	{
		uiProgressBarSetValue( b, luaL_checkinteger( L, 2 ) );
	}
	return 0;
}

static luaL_Reg progress_functions[] =
{
	{ "SetValue", progress_setvalue },
	{ 0, 0 }
};

static int new_progress( lua_State* L )
{
	control_create( L, (uiControl*) uiNewProgressBar(), control_progressbar, progress_functions );
	return 1;
}


static int slider_value( lua_State* L )
{
	uiSlider* s = (uiSlider*) check_control( L, 1, control_slider );
	if( s )
	{
		lua_pushinteger( L, uiSliderValue( s ) );
		return 1;
	}
	return 0;
}

static int slider_setvalue( lua_State* L )
{
	uiSlider* s = (uiSlider*) check_control( L, 1, control_slider );
	if( s )
	{
		uiSliderSetValue( s, luaL_checkinteger( L, 2 ) );
	}
	return 0;
}

static void slider_changed_callback( uiSlider* s, void* d )
{
	invoke_callback( (lua_State*) d, s, callback_changed );
}

static int slider_onchanged( lua_State* L )
{
	uiSlider* s = (uiSlider*) check_control( L, 1, control_slider );
	if( s )
	{
		register_callback( L, 2, s, callback_changed );
		if( lua_isfunction( L, 2 ) )
			uiSliderOnChanged( s, slider_changed_callback, L );
		else
			uiSliderOnChanged( s, NULL, NULL );
	}
	return 0;
}

static luaL_Reg slider_functions[] =
{
	{ "Value", slider_value },
	{ "SetValue", slider_setvalue },
	{ "OnChanged", slider_onchanged },
	{ 0, 0 }
};

static int new_slider( lua_State* L )
{
	control_create( L, (uiControl*) uiNewSlider( luaL_optinteger( L, 1, 0 ), luaL_optinteger( L, 2, 100 ) ), control_slider, slider_functions );
	return 1;
}


static int new_hseparator( lua_State* L )
{
	control_create( L, (uiControl*) uiNewHorizontalSeparator(), control_separator, 0 );
	return 1;
}


static int combobox_append( lua_State* L )
{
	uiCombobox* s = (uiCombobox*) check_control( L, 1, control_combobox );
	if( s )
	{
		uiComboboxAppend( s, luaL_checkstring( L, 2 ) );
	}
	return 0;
}

static int combobox_selected( lua_State* L )
{
	uiCombobox* s = (uiCombobox*) check_control( L, 1, control_combobox );
	if( s )
	{
		lua_pushinteger( L, uiComboboxSelected( s ) );
		return 1;
	}
	return 0;
}

static int combobox_setselected( lua_State* L )
{
	uiCombobox* s = (uiCombobox*) check_control( L, 1, control_combobox );
	if( s )
	{
		uiComboboxSetSelected( s, luaL_checkinteger( L, 2 ) );
	}
	return 0;
}

static void combobox_selected_callback( uiCombobox* c, void* d )
{
	invoke_callback( (lua_State*) d, c, callback_selected );
}

static int combobox_onselected( lua_State* L )
{
	uiCombobox* s = (uiCombobox*) check_control( L, 1, control_combobox );
	if( s )
	{
		register_callback( L, 2, s, callback_selected );
		if( lua_isfunction( L, 2 ) )
			uiComboboxOnSelected( s, combobox_selected_callback, L );
		else
			uiComboboxOnSelected( s, NULL, NULL );
	}
	return 0;
}

static luaL_Reg combobox_functions[] =
{
	{ "Append", combobox_append },
	{ "Selected", combobox_selected },
	{ "SetSelected", combobox_setselected },
	{ "OnSelected", combobox_onselected },
	{ 0, 0 }
};

static int new_combobox( lua_State* L )
{
	if( lua_toboolean( L, 1 ) == 1 )
	{
		control_create( L, (uiControl*) uiNewEditableCombobox(), control_combobox, combobox_functions );
	}
	else
	{
		control_create( L, (uiControl*) uiNewCombobox(), control_combobox, combobox_functions );
	}
	return 1;
}


static int radiobuttons_append( lua_State* L )
{
	uiRadioButtons* s = (uiRadioButtons*) check_control( L, 1, control_radiobuttons );
	if( s )
	{
		uiRadioButtonsAppend( s, luaL_checkstring( L, 2 ) );
	}
	return 0;
}

static luaL_Reg radiobuttons_functions[] =
{
	{ "Append", radiobuttons_append },
	{ 0, 0 }
};

static int new_radiobuttons( lua_State* L )
{
	control_create( L, (uiControl*) uiNewRadioButtons(), control_radiobuttons, radiobuttons_functions );
	return 1;
}


static luaL_Reg datetimepicker_functions[] =
{
	{ 0, 0 }
};

static int new_datetimepicker( lua_State* L )
{
	control_create( L, (uiControl*) uiNewDateTimePicker(), control_datetimepicker, datetimepicker_functions );
	return 1;
}

static int new_timepicker( lua_State* L )
{
	control_create( L, (uiControl*) uiNewTimePicker(), control_datetimepicker, datetimepicker_functions );
	return 1;
}

static int new_datepicker( lua_State* L )
{
	control_create( L, (uiControl*) uiNewDatePicker(), control_datetimepicker, datetimepicker_functions );
	return 1;
}


static int multilineentry_text( lua_State* L )
{
	uiMultilineEntry* m = (uiMultilineEntry*) check_control( L, 1, control_multilineentry );
	if( m )
	{
		lua_pushstring( L, uiMultilineEntryText( m ) );
		return 1;
	}
	return 0;
}

static int multilineentry_settext( lua_State* L )
{
	uiMultilineEntry* m = (uiMultilineEntry*) check_control( L, 1, control_multilineentry );
	if( m )
	{
		uiMultilineEntrySetText( m, luaL_checkstring( L, 2 ) );
	}
	return 0;
}

static int multilineentry_append( lua_State* L )
{
	uiMultilineEntry* m = (uiMultilineEntry*) check_control( L, 1, control_multilineentry );
	if( m )
	{
		uiMultilineEntryAppend( m, luaL_checkstring( L, 2 ) );
	}
	return 0;
}

static int multilineentry_readonly( lua_State* L )
{
	uiMultilineEntry* m = (uiMultilineEntry*) check_control( L, 1, control_multilineentry );
	if( m )
	{
		lua_pushboolean( L, uiMultilineEntryReadOnly( m ) );
		return 1;
	}
	return 0;
}

static int multilineentry_setreadonly( lua_State* L )
{
	uiMultilineEntry* m = (uiMultilineEntry*) check_control( L, 1, control_multilineentry );
	if( m )
	{
		uiMultilineEntrySetReadOnly( m, lua_toboolean( L, 2 ) );
	}
	return 0;
}

static void multilineentry_changed_callback( uiMultilineEntry* m, void* d )
{
	invoke_callback( (lua_State*) d, m, callback_changed );
}

static int multilineentry_onchanged( lua_State* L )
{
	uiMultilineEntry* m = (uiMultilineEntry*) check_control( L, 1, control_multilineentry );
	if( m )
	{
	    register_callback( L, 2, m, callback_changed );
	    if( lua_isfunction( L, 2 ) )
            uiMultilineEntryOnChanged( m, multilineentry_changed_callback, L );
        else
            uiMultilineEntryOnChanged( m, 0, 0 );
	}
	return 0;
}

static luaL_Reg multilineentry_functions[] =
{
    { "Text", multilineentry_text },
    { "SetText", multilineentry_settext },
    { "Append", multilineentry_append },
    { "ReadOnly", multilineentry_readonly },
    { "SetReadOnly", multilineentry_setreadonly },
    { "OnChanged", multilineentry_onchanged },
	{ 0, 0 }
};

static int new_multilineentry( lua_State* L )
{
	control_create( L, (uiControl*) uiNewMultilineEntry(), control_multilineentry, multilineentry_functions );
	return 1;
}


int menuitem_enable( lua_State* L )
{
	uiMenuItem* m = (uiMenuItem*) check_control( L, 1, control_menuitem );
	if( m )
	{
		uiMenuItemEnable( m );
	}
	return 0;
}

int menuitem_disable( lua_State* L )
{
	uiMenuItem* m = (uiMenuItem*) check_control( L, 1, control_menuitem );
	if( m )
	{
		uiMenuItemDisable( m );
	}
	return 0;
}

int menuitem_checked( lua_State* L )
{
	uiMenuItem* m = (uiMenuItem*) check_control( L, 1, control_menuitem );
	if( m )
	{
		lua_pushboolean( L, uiMenuItemChecked( m ) );
		return 1;
	}
	return 0;
}

int menuitem_setchecked( lua_State* L )
{
	uiMenuItem* m = (uiMenuItem*) check_control( L, 1, control_menuitem );
	if( m )
	{
		uiMenuItemSetChecked( m, lua_toboolean( L, 2 ) );
	}
	return 0;
}

void menuitem_clicked_callback( uiMenuItem* i, uiWindow* w, void* d )
{
	invoke_callback( (lua_State*) d, i, callback_clicked );
}

int menuitem_onclicked( lua_State* L )
{
	uiMenuItem* m = (uiMenuItem*) check_control( L, 1, control_menuitem );
	if( m )
	{
		register_callback( L, 2, m, callback_clicked );
		if( lua_isfunction( L, 2 ) )
			uiMenuItemOnClicked( m, menuitem_clicked_callback, L );
		else
			uiMenuItemOnClicked( m, NULL, NULL );
	}
	return 0;
}

static luaL_Reg menuitem_functions[] =
{
	{ "Enable", menuitem_enable },
	{ "Disable", menuitem_disable },
	{ "Checked", menuitem_checked },
	{ "SetChecked", menuitem_setchecked },
	{ "OnClicked", menuitem_onclicked },
	{ 0, 0 }
};

int menu_append( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_menu );
	if( m )
	{
		uiMenuItem* i = uiMenuAppendItem( m, luaL_checkstring( L, 2 ) );
		control_create( L, (uiControl*) i, control_menuitem, menuitem_functions );
		return 1;
	}
	return 0;
}

int menu_appendcheckitem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_menu );
	if( m )
	{
		uiMenuItem* i = uiMenuAppendCheckItem( m, luaL_checkstring( L, 2 ) );
		control_create( L, (uiControl*) i, control_menuitem, menuitem_functions );
		return 1;
	}
	return 0;
}

int menu_appendquititem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_menu );
	if( m )
	{
		uiMenuItem* i = uiMenuAppendQuitItem( m );
		control_create( L, (uiControl*) i, control_menuitem, menuitem_functions );
		return 1;
	}
	return 0;
}

int menu_appendpreferencesitem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_menu );
	if( m )
	{
		uiMenuItem* i = uiMenuAppendPreferencesItem( m );
		control_create( L, (uiControl*) i, control_menuitem, menuitem_functions );
		return 1;
	}
	return 0;
}

int menu_appendaboutitem( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_menu );
	if( m )
	{
		uiMenuItem* i = uiMenuAppendAboutItem( m );
		control_create( L, (uiControl*) i, control_menuitem, menuitem_functions );
		return 1;
	}
	return 0;
}

int menu_appendseparator( lua_State* L )
{
	uiMenu* m = (uiMenu*) check_control( L, 1, control_menu );
	if( m )
	{
		uiMenuAppendSeparator( m );
	}
	return 0;
}

static luaL_Reg menu_functions[] =
{
	{ "AppendItem", menu_append },
	{ "AppendCheckItem", menu_appendcheckitem },
	{ "AppendQuitItem", menu_appendquititem },
	{ "AppendPreferencesItem", menu_appendpreferencesitem },
	{ "AppendAboutItem", menu_appendaboutitem },
	{ 0, 0 }
};

static int new_menu( lua_State* L )
{
	uiControl* c = (uiControl*) uiNewMenu( luaL_optlstring( L, 1, "Menu", NULL ) );
	printf( "new menu\n" );
	
	control_create( L, c, control_menu, menu_functions );
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

	control_create( L, (uiControl*) a, control_label, area_functions );
	return 1;
}

static luaL_Reg fontbutton_functions[] =
{
	{ 0, 0 }
};

static int new_fontbutton( lua_State* L )
{
	control_create( L, (uiControl*) uiNewFontButton(), control_fontbutton, fontbutton_functions );
	return 1;
}

static luaL_Reg colorbutton_functions[] =
{
	{ 0, 0 }
};

static int new_colorbutton( lua_State* L )
{
	control_create( L, (uiControl*) uiNewColorButton(), control_colorbutton, colorbutton_functions );
	return 1;
}


static int openfile( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_window );
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
	uiWindow* w = (uiWindow*) check_control( L, 1, control_window );
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
	uiWindow* w = (uiWindow*) check_control( L, 1, control_window );
	if( w )
	{
		uiMsgBox( w, luaL_checkstring( L, 2 ), luaL_checkstring( L, 3 ) );
	}
	return 0;
}

static int msgboxerror( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_control( L, 1, control_window );
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
