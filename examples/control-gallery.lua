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
            local w = ui.NewWindow( "About", 320, 120 ):SetMargined( true ):SetIcon( ui.NewImage( "examples/lua.gif" ) )
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

window:SetIcon( ui.NewImage( "examples/lua.gif" ) )
	
local box = ui.NewVBox():SetPadded( true )
window:SetChild( box )

local hbox = ui.NewHBox():SetPadded( true )
box:Append( hbox, true )

local group = ui.NewGroup( "Basic Controls" ):SetMargined( true )
hbox:Append( group )

local inner = ui.NewVBox():SetPadded( true )
group:SetChild( inner )
    inner
        :Append( ui.NewButton( "Button" ):SetIcon( ui.NewImage( "examples/test.png" ):Resize( 21, 21 ) ) )
        :Append( ui.NewCheckbox( "Checkbox" ) )
        :Append( ui.NewEntry( "Entry" ) )
        :Append( ui.NewLabel( "Label" ) )
		:Append( ui.NewImageBox( ui.NewImage():Load( "examples/test.png" ):Resize( 64, 64 ) ) )
        :Append( ui.NewHSeparator() )
        :Append( ui.NewDatePicker() )
        :Append( ui.NewTimePicker() )
        :Append( ui.NewDateTimePicker() )
        :Append( ui.NewFontButton() )
        :Append( ui.NewColorButton():OnChanged( function( s, r, g, b, a ) print( "new colour " .. r .. "," .. g .. "," .. b .. "," .. a ) end ) )
        :Append( ui.NewMultilineEntry():SetText( "Hello\r\nWorld" ), true )
        :Append( ui.NewButton( "Button 2" ):OnClicked( function( b ) ui.MsgBox( window, "Button 2", b:Text() ) b:SetText( "Click!" ) b:Disable() end ) )

local inner2 = ui.NewVBox()
inner2:SetPadded( true )
hbox:Append( inner2, true )

local group = ui.NewGroup( "Numbers" )
group:SetMargined( true )
inner2:Append( group )

local inner = ui.NewVBox()
inner:SetPadded( true )
group:SetChild( inner )

local spinbox = ui.NewSpinbox( 0, 100 )
inner:Append( spinbox )

local slider = ui.NewSlider( 0, 100 )

inner:Append( slider )

local progress = ui.NewProgressBar()
inner:Append( progress )

spinbox:OnChanged( function( sender, value )
	slider:SetValue( value )
	progress:SetValue( value )
end )

slider:OnChanged( function( sender, value )
	spinbox:SetValue( value )
	progress:SetValue( value )
end )

local group = ui.NewGroup( "Lists" )
group:SetMargined( true )
inner2:Append( group )

local inner = ui.NewVBox()
inner:SetPadded( true )
group:SetChild( inner )

inner:Append( ui.NewCombobox()
             :Append( "Combobox Item 1" )
             :Append( "Combobox Item 2" )
             :Append( "Combobox Item 3" )
             :SetSelected( 0 )
             :OnSelected( function( sender, index ) ui.MsgBox( window, "Selected", tostring(index) ) end )
            )
inner:Append( ui.NewEditableCombobox( true )
             :Append( "Editable Item 1" )
             :Append( "Editable Item 2" )
             :Append( "Editable Item 3" )
			 :OnChanged( function( sender, item ) ui.MsgBox( window, "Changed", item ) end )
			)
inner:Append( ui.NewRadioButtons()
             :Append( "Radio Button 1" )
             :Append( "Radio Button 2" )
             :Append( "Radio Button 3" ) )

local tab = ui.NewTab()
tab:Append( "Page 1", ui.NewHBox():Append( ui.NewMultilineEntry():SetText( "Multiline\r\nText" ), true ) )
tab:Append( "Page 2", ui.NewHBox() )
tab:Append( "Page 3", ui.NewHBox() )
inner2:Append( tab, true )


ui.ShouldQuit( function()
	window:Destroy()
    ui.Quit()
end )

window:Show()
ui.Main()
