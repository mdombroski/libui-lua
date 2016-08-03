local ui = require "libui.core"

local menu = ui.NewMenu( "File" )
menu:AppendItem( "Open" )
        :OnClicked( function( m, w )
            local filename = ui.OpenFile( w )
            w:SetTitle( filename or "" )
            if not filename then
                ui.MsgBoxError( w, "No file selected", "Don't be alarmed!" )
            else
                ui.MsgBox( w, "File selected", filename )
            end
        end )
menu:AppendItem( "Save" )
        :OnClicked( function( m, w )
            local filename = ui.SaveFile( w )
            if not filename then
                ui.MsgBoxError( w, "No file selected", "Don't be alarmed!" )
            else
                ui.MsgBox( w, "File selected (don't worry, it's still there)", filename )
            end
        end )
menu:AppendQuitItem()


local menu = ui.NewMenu( "Edit" )
menu:AppendCheckItem( "Checkable Item" )
menu:AppendSeparator()
menu:AppendItem( "Disabled Item" ):Disable()
menu:AppendPreferencesItem()

local menu = ui.NewMenu( "Help" )
menu:AppendItem( "Help" )
menu:AppendAboutItem()
        :OnClicked( function()
            local w = ui.NewWindow( "About", 320, 120 ):SetMargined( true )
            w:SetChild(
                ui.NewVBox()
                    :Append( ui.NewHBox():Append( ui.NewLabel( "LibUI Widget Gallery (Lua)\r\n\r\nAuthor: M. Dombroski" ), true ), true )
                    :Append( ui.NewHBox()
                        :Append( ui.NewHBox(), true )
                        :Append( ui.NewButton( "Close" ):OnClicked( function() w:Destroy() end ), true )
                        :Append( ui.NewHBox(), true ) )
            ):Show()
        end )


local window = ui.NewWindow( "libui Control Gallery", 640, 480, true )
    :SetMargined( true )
window:OnClosing( function()
        window:Destroy()
        ui.Quit()
    end )


-- Basic controls category

local basic = ui.NewVBox( true )
basic:Append( ui.NewHBox( true ):Append( ui.NewButton( "Button" ) ):Append( ui.NewCheckbox( "Checkbox" ) ) )
basic:Append( ui.NewLabel( "This is a label. Right now, labels can only span one line." ) )
basic:Append( ui.NewHSeparator() )
basic:Append( ui.NewGroup( "Entries" ):SetMargined( true ):SetChild( ui.NewForm():SetPadded( true )
		:Append( "Entry", ui.NewEntry() )
		:Append( "Password", ui.NewPasswordEntry() )
		:Append( "Search", ui.NewSearchEntry() )
		:Append( "Multiline", ui.NewMultilineEntry(), true )
		:Append( "Multiline (no wrap)", ui.NewNonWrappingMultilineEntry(), true ) ) )


-- Lists and Choosers category

local lists = ui.NewHBox( true )

local spinbox = ui.NewSpinbox( 0, 100 )
local slider = ui.NewSlider( 0, 100 )
local progress = ui.NewProgressBar( 0 )
spinbox:OnChanged( function( s, v ) slider:SetValue( v ) progress:SetValue( v ) end )
slider:OnChanged( function( s, v ) spinbox:SetValue( v ) progress:SetValue( v ) end )

lists:Append( ui.NewGroup( "Numbers" ):SetMargined( true ):SetChild( ui.NewVBox( true )
		:Append( spinbox ):Append( slider ):Append( progress )
		:Append( ui.NewProgressBar( -1 ) ) ), true )
lists:Append( ui.NewGroup( "Lists" ):SetMargined( true ):SetChild( ui.NewVBox( true )
		:Append( ui.NewCombobox():Append( "Conbobox Item 1" ):Append( "Combobox Item 2 " ):Append( "Combobox Item 3" ) )
		:Append( ui.NewEditableCombobox():Append( "Editable Item 1" ):Append( "Editable Item 2" ):Append( "Editable Item 3" ) )
		:Append( ui.NewRadioButtons():Append( "Radio Button 1" ):Append( "Radio Button 2" ):Append( "Radio Button 3" ) ) ), true )


-- Choosers category

local choosers = ui.NewHBox( true )
choosers:Append( ui.NewVBox( true )
		:Append( ui.NewDatePicker() )
		:Append( ui.NewTimePicker() )
		:Append( ui.NewDateTimePicker() )
		:Append( ui:NewFontButton() )
		:Append( ui:NewColorButton() ) )
choosers:Append( ui.NewHSeparator() )
choosers:Append( ui.NewVBox( true ):Append( ui.NewGrid( true )
		:Append( ui.NewButton( "Open File" ), 0, 0 )
		:Append( ui.NewEntry():SetReadOnly( true ), 1, 0, 1, 1, 1 )
		:Append( ui.NewButton( "Save File" ), 0, 1 )
		:Append( ui.NewEntry():SetReadOnly( true ), 1, 1, 1, 1, 1 ) ), true )


local tab = ui.NewTab()
tab:Append( "Basic Controls", basic, true )
tab:Append( "Numbers and Lists", lists, true )
tab:Append( "Data Choosers", choosers, true )
window:SetChild( tab )


ui.ShouldQuit( function()
	window:Destroy()
    ui.Quit()
end )

window:Show()
ui.Main()
