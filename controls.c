#include "controls.h"

#include "object.h"
#include "callback.h"
#include "control-common.h"
#include "image.h"
#include "draw.h"

#include <lua.h>
#include <lauxlib.h>

#include <ui.h>


static int callback_uiWindowOnClosing( uiWindow* w, void* d )
{
    invoke_callback( (lua_State*) d, w, callback_OnClosing, 0 );
    return 0;
}

static int l_uiWindowTitle( lua_State* L )
{
	char* s = uiWindowTitle( (uiWindow*) check_object( L, 1, uiWindowSignature ) );
	lua_pushstring( L, s );
	uiFreeText( s );
	return 1;
}

static int l_uiWindowSetIcon( lua_State* L )
{
	uiWindowSetIcon( (uiWindow*) check_object( L, 1, uiWindowSignature ), check_image( L, 2 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

//DECLARE_GETTER( uiWindow, Title, string )
DECLARE_SETTER( uiWindow, SetTitle, string )
DECLARE_GETTER_2( uiWindow, Position, integer, integer )
DECLARE_SETTER_2( uiWindow, SetPosition, integer, integer )
DECLARE_ACTION( uiWindow, Center )
DECLARE_GETTER_2( uiWindow, ContentSize, integer, integer )
DECLARE_SETTER_2( uiWindow, SetContentSize, integer, integer )
DECLARE_GETTER( uiWindow, Fullscreen, boolean )
DECLARE_SETTER( uiWindow, SetFullscreen, boolean )
DECLARE_GETTER( uiWindow, Borderless, boolean )
DECLARE_SETTER( uiWindow, SetBorderless, boolean )
DECLARE_GETTER( uiWindow, Margined, boolean)
DECLARE_SETTER( uiWindow, SetMargined, boolean )
DECLARE_CALLBACK_REGISTER( uiWindow, uiWindowSignature, OnClosing )
DECLARE_CALLBACK( uiWindow, OnPositionChanged )
DECLARE_CALLBACK( uiWindow, OnContentSizeChanged )

static int window_setchild( lua_State* L )
{
	uiWindow* w = (uiWindow*) check_object( L, 1, uiWindowSignature );
	uiControl* c = (uiControl*) check_control( L, 2, 0 );
	uiWindowSetChild( w, c );
	lua_pushvalue( L, 1 );
	return 1;
}


static luaL_Reg window_functions[] =
{
	{ "Title", l_uiWindowTitle },
	{ "SetTitle", l_uiWindowSetTitle },
	{ "SetIcon", l_uiWindowSetIcon },
	{ "Position", l_uiWindowPosition },
	{ "SetPosition", l_uiWindowPosition },
	{ "Center", l_uiWindowCenter },
	{ "OnPositionChanged", l_uiWindowOnPositionChanged },
	{ "ContentSize", l_uiWindowContentSize },
	{ "SetContentSize", l_uiWindowSetContentSize },
	{ "OnContentSizeChanged", l_uiWindowOnContentSizeChanged },
	{ "Fullscreen", l_uiWindowFullscreen },
	{ "SetFullscreen", l_uiWindowSetFullscreen },
	{ "Borderless", l_uiWindowBorderless },
	{ "SetBorderless", l_uiWindowSetBorderless },
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


static int l_uiButtonText( lua_State* L )
{
	char* s = uiButtonText( (uiButton*) check_object( L, 1, uiButtonSignature ) );
	lua_pushstring( L, s );
	return 1;
}

static int l_uiButtonSetIcon( lua_State* L )
{
	uiButtonSetIcon( (uiButton*) check_object( L, 1, uiButtonSignature ), check_image( L, 2 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

//DECLARE_GETTER( uiButton, Text, string )
DECLARE_SETTER( uiButton, SetText, string )
DECLARE_CALLBACK( uiButton, OnClicked )

static luaL_Reg button_functions[] =
{
	{ "Text", l_uiButtonText },
	{ "SetText", l_uiButtonSetText },
	{ "SetIcon", l_uiButtonSetIcon },
	{ "OnClicked", l_uiButtonOnClicked },
	{ 0, 0 }
};

static int new_button( lua_State* L )
{
	uiButton* b = uiNewButton( luaL_optlstring( L, 1, "Button", NULL ) );
	object_create( L, b, uiButtonSignature, control_common, button_functions, 0 );

	return 1;
}


static int l_uiBoxAppend( lua_State* L )
{
	uiBox* b = (uiBox*) check_object( L, 1, uiBoxSignature );
	uiControl* o = (uiControl*) check_control( L, 2, 0 );
	uiBoxAppend( b, o, lua_toboolean( L, 3 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

DECLARE_GETTER( uiBox, Padded, boolean )
DECLARE_SETTER( uiBox, SetPadded, boolean )
DECLARE_SETTER( uiBox, Delete, integer )

static luaL_Reg box_functions[] =
{
	{ "Append", l_uiBoxAppend },
	{ "Delete", l_uiBoxDelete },
	{ "Padded", l_uiBoxPadded },
	{ "SetPadded", l_uiBoxSetPadded },
	{ 0, 0 }
};

static int new_hbox( lua_State* L )
{
	uiBox* b = uiNewHorizontalBox();
	object_create( L, b, uiBoxSignature, control_common, box_functions, 0 );
	if( lua_isboolean( L, 1 ) )
	{
		uiBoxSetPadded( b, lua_toboolean( L, 1 ) );
	}
	return 1;
}

static int new_vbox( lua_State* L )
{
	uiBox* b = uiNewVerticalBox();
	object_create( L, b, uiBoxSignature, control_common, box_functions, 0 );
	if( lua_isboolean( L, 1 ) )
	{
		uiBoxSetPadded( b, lua_toboolean( L, 1 ) );
	}
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

static luaL_Reg checkbox_functions[] =
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


static int l_uiEntryText( lua_State* L )
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

static luaL_Reg entry_functions[] =
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

static int new_password_entry( lua_State* L )
{
	object_create( L, uiNewPasswordEntry(), uiEntrySignature, control_common, entry_functions, 0 );
	return 1;
}

static int new_search_entry( lua_State* L )
{
	object_create( L, uiNewSearchEntry(), uiEntrySignature, control_common, entry_functions, 0 );
	return 1;
}


static int l_uiLabelText( lua_State* L )
{
	char* s = uiLabelText( (uiLabel*) check_object( L, 1, uiLabelSignature ) );
	lua_pushstring( L, s );
	uiFreeText( s );
	return 1;
}

//DECLARE_GETTER( uiLabel, Text, string )
DECLARE_SETTER( uiLabel, SetText, string )

static luaL_Reg label_functions[] =
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

static int l_uiImageBoxSetImage( lua_State* L )
{
	uiImageBox* i = (uiImageBox*) check_object( L, 1, uiImageBoxSignature );
	
	if( lua_isnoneornil( L, 2 ) )
	{
		uiImageBoxSetImage( i, 0 );
	}
	else
	{
		uiImageBoxSetImage( i, check_image( L, 2 ) );
	}

	lua_pushvalue( L, 1 );
	return 1;
}

static luaL_Reg imagebox_functions[] =
{
	{ "SetImage", l_uiImageBoxSetImage },
	{ 0, 0 }
};

static int new_imagebox( lua_State* L )
{
	// image as parameter
	uiImage *i = NULL;
	if( ! lua_isnoneornil( L, 1 ) )
	{
		i = check_image( L, 1 );
	}

	uiImageBox* b = uiNewImageBox( i );
	object_create( L, b, uiImageBoxSignature, control_common, imagebox_functions, 0 );
	return 1;
}


static int tab_append( lua_State* L )
{
	uiTab* t = (uiTab*) check_object( L, 1, uiTabSignature );
	uiControl* c = check_control( L, 3, 0 );
	uiTabAppend( t, luaL_checkstring( L, 2 ), c );
	if( lua_isboolean( L, 4 ) )
	{
		uiTabSetMargined( t, uiTabNumPages( t ) - 1, lua_toboolean( L, 4 ) );
	}
	lua_pushvalue( L, 1 );
	return 1;
}

static int tab_insert( lua_State* L )
{
	uiTab* t = (uiTab*) check_object( L, 1, uiTabSignature );
	uiControl* c = check_control( L, 4, 0 );
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


static int l_uiGroupTitle( lua_State* L )
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
	uiControl* c = check_control( L, 2, 0 );
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


DECLARE_GETTER( uiProgressBar, Value, integer )
DECLARE_SETTER( uiProgressBar, SetValue, integer )

static luaL_Reg progress_functions[] =
{
	{ "Value", l_uiProgressBarValue },
	{ "SetValue", l_uiProgressBarSetValue },
	{ 0, 0 }
};

static int new_progress( lua_State* L )
{
	uiProgressBar* p = uiNewProgressBar();
	object_create( L, p, uiProgressBarSignature, control_common, progress_functions, 0 );
	if( lua_isinteger( L, 1 ) )
	{
		uiProgressBarSetValue( p, lua_tointeger( L, 1 ) );
	}
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

static int new_vseparator( lua_State* L )
{
	object_create( L, uiNewVerticalSeparator(), uiSeparatorSignature, control_common, 0 );
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
	object_create( L, uiNewCombobox(), uiComboboxSignature, control_common, combobox_functions, 0 );
	return 1;
}


static void callback_uiEditableComboboxOnChanged( uiEditableCombobox* c, void* d )
{
	lua_State* L = (lua_State*) d;
	if( L )
	{
		object_create( L, uiNewCombobox(), uiComboboxSignature, control_common, combobox_functions, 0 );
		char* text = uiEditableComboboxText( c );
		lua_pushstring( L, text );
		invoke_callback( L, c, callback_OnChanged, 1 );
		uiFreeText( text );
	}
}

static int l_uiEditableComboboxText( lua_State* L )
{
	char* s = uiEditableComboboxText( (uiEditableCombobox*) check_object( L, 1, uiEditableComboboxSignature ) );
	lua_pushstring( L, s );
	uiFreeText( s );
	return 1;
}

DECLARE_SETTER( uiEditableCombobox, SetText, string )
DECLARE_SETTER( uiEditableCombobox, Append, string )
DECLARE_CALLBACK_REGISTER( uiEditableCombobox, uiEditableComboboxSignature, OnChanged )

static luaL_Reg editable_combobox_functions[] =
{
	{ "Text", l_uiEditableComboboxText },
	{ "SetText", l_uiEditableComboboxSetText },
	{ "Append", l_uiEditableComboboxAppend },
	{ "OnChanged", l_uiEditableComboboxOnChanged },
	{ 0, 0 }
};

static int new_editablecombobox( lua_State* L )
{
	object_create( L, uiNewEditableCombobox(), uiEditableComboboxSignature, control_common, editable_combobox_functions, 0 );
	return 1;
}


DECLARE_SETTER( uiRadioButtons, Append, string )
DECLARE_GETTER( uiRadioButtons, Selected, boolean )
DECLARE_SETTER( uiRadioButtons, SetSelected, boolean )
DECLARE_CALLBACK( uiRadioButtons, OnSelected )

static luaL_Reg radiobuttons_functions[] =
{
	{ "Append", l_uiRadioButtonsAppend },
	{ "Selected", l_uiRadioButtonsSelected },
	{ "SetSelected", l_uiRadioButtonsSetSelected },
	{ "OnSelected", l_uiRadioButtonsOnSelected },
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


static int l_uiMultilineEntryText( lua_State* L )
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

static int new_nonwrapping_multilineentry( lua_State* L )
{
	object_create( L, uiNewNonWrappingMultilineEntry(), uiMultilineEntrySignature, control_common, multilineentry_functions, 0 );
	return 1;
}


static int l_uiFontButtonFont( lua_State* L )
{
	uiFontButton* fb = check_object( L, 1, uiFontButtonSignature );
	uiDrawTextFont* f = uiFontButtonFont( fb );

	return draw_new_font( L, f );
}

DECLARE_CALLBACK( uiFontButton, OnChanged )

static luaL_Reg fontbutton_functions[] =
{
	{ "Font", l_uiFontButtonFont },
	{ "OnChanged", l_uiFontButtonOnChanged },
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


int l_uiFormAppend( lua_State* L )
{
	uiFormAppend( 
		(uiForm*) check_object( L, 1, uiFormSignature ),
		luaL_checkstring( L, 2 ),
		check_control( L, 3, 0 ),
		lua_toboolean( L, 4 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

DECLARE_GETTER( uiForm, Padded, boolean )
DECLARE_SETTER( uiForm, SetPadded, boolean )
DECLARE_SETTER( uiForm, Delete, integer )

static luaL_Reg form_functions[] =
{
	{ "Append", l_uiFormAppend },
	{ "Delete", l_uiFormDelete },
	{ "Padded", l_uiFormPadded },
	{ "SetPadded", l_uiFormSetPadded },
	{ 0, 0 }
};

static int new_form( lua_State* L )
{
	uiForm* f = uiNewForm();
	object_create( L, f, uiFormSignature, control_common, form_functions, 0 );
	if( lua_isboolean( L, 1 ) )
	{
		uiFormSetPadded( f, lua_toboolean( L, 1 ) );
	}
	return 1;
}


int l_uiGridAppend( lua_State* L )
{
	uiGrid* g = (uiGrid*) check_object( L, 1, uiGridSignature );
	uiControl* c = check_control( L, 2, 0 );
	int left = luaL_checkinteger( L, 3 );
	int top = luaL_checkinteger( L, 4 );
	int xspan = luaL_optinteger( L, 5, 1 );
	int yspan = luaL_optinteger( L, 6, 1 );
	int hexpand = luaL_optinteger( L, 7, 1 );
	uiAlign halign = luaL_optinteger( L, 8, uiAlignFill );
	int vexpand = luaL_optinteger( L, 9, 1 );
	uiAlign valign = luaL_optinteger( L, 10, uiAlignFill );
	uiGridAppend( g, c, left, top, xspan, yspan, hexpand, halign, vexpand, valign );
	lua_pushvalue( L, 1 );
	return 1;
}

int l_uiGridInsertAt( lua_State* L )
{
	return 1;
}

DECLARE_GETTER( uiGrid, Padded, boolean )
DECLARE_SETTER( uiGrid, SetPadded, boolean )

static luaL_Reg grid_functions[] =
{
	{ "Append", l_uiGridAppend },
	{ "InsertAt", l_uiGridInsertAt },
	{ "Padded", l_uiGridPadded },
	{ "SetPadded", l_uiGridSetPadded },
	{ 0, 0 }
};

static int new_grid( lua_State* L )
{
	uiGrid* g = uiNewGrid();
	object_create( L, g, uiGridSignature, control_common, grid_functions, 0 );
	if( lua_isboolean( L, 1 ) )
	{
		uiGridSetPadded( g, lua_toboolean( L, 1 ) );
	}

	return 1;
}

luaL_Reg controls_functions[] =
{
	{ "NewWindow", new_window },
	{ "NewButton", new_button },
	{ "NewHBox", new_hbox },
	{ "NewVBox", new_vbox },
	{ "NewEntry", new_entry },
	{ "NewPasswordEntry", new_password_entry },
	{ "NewSearchEntry", new_search_entry },
	{ "NewCheckbox", new_checkbox },
	{ "NewLabel", new_label },
	{ "NewImageBox", new_imagebox },
	{ "NewTab", new_tab },
	{ "NewGroup", new_group },
	{ "NewSpinbox", new_spinbox },
	{ "NewProgressBar", new_progress },
	{ "NewSlider", new_slider },
	{ "NewHSeparator", new_hseparator },
	{ "NewVSeparator", new_vseparator },
	{ "NewCombobox", new_combobox },
	{ "NewEditableCombobox", new_editablecombobox },
	{ "NewRadioButtons", new_radiobuttons },
	{ "NewDateTimePicker", new_datetimepicker },
	{ "NewTimePicker", new_timepicker },
	{ "NewDatePicker", new_datepicker },
	{ "NewMultilineEntry", new_multilineentry },
	{ "NewNonWrappingMultilineEntry", new_nonwrapping_multilineentry },
	{ "NewFontButton", new_fontbutton },
	{ "NewColorButton", new_colorbutton },
	{ "NewColourButton", new_colorbutton },
	{ "NewForm", new_form },
	{ "NewGrid", new_grid },
	{ 0, 0 }
};

