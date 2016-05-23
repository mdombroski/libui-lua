local ui = require "libui.core"

local menu = ui.NewMenu( "File" )
local openitem = menu:AppendItem( "Open" )
local saveitem = menu:AppendItem( "Save" )
local item = menu:AppendQuitItem()

local menu = ui.NewMenu( "Edit" )
local item = menu:AppendCheckItem( "Checkable Item" )
menu:AppendSeparator()
local item = menu:AppendItem( "Disabled Item" ):Disable()
local item = menu:AppendPreferencesItem()

local menu = ui.NewMenu( "Help" )
local item = menu:AppendItem( "Help" )
local item = menu:AppendAboutItem()



local window = ui.NewWindow( "libui Control Gallery", 640, 480, 1 )


openitem:OnClicked( function()
    local filename = ui.OpenFile( window )
	if not filename == nil then
		ui.MsgBoxError( window, "No file selected", "Don't be alarmed!" )
	else
		ui.MsgBox( window, "File selected", filename )
	end
end )
saveitem:OnClicked( function()
	local filename = ui.SaveFile( window )
	if not filename == nil then
		ui.MsgBoxError( window, "No file selected", "Don't be alarmed!" )
	else
		ui.MsgBox( window, "File selected (don't worry, it's still there)", filename )
	end
end )


ui.ShouldQuit( function()
	window:Destroy()
end )


window:SetMargined( true )
window:OnClosing( function()
	window:Destroy()
	ui.Quit()
end )

local box = ui.NewVBox():SetPadded( true )
window:SetChild( box )

local hbox = ui.NewHBox():SetPadded( true )
box:Append( hbox, true )

local group = ui.NewGroup( "Basic Controls" ):SetMargined( true )
hbox:Append( group )

local inner = ui.NewVBox():SetPadded( true )
group:SetChild( inner )
    inner
        :Append( ui.NewButton( "Button" ) )
        :Append( ui.NewCheckbox( "Checkbox" ) )
        :Append( ui.NewEntry( "Entry" ) )
        :Append( ui.NewLabel( "Label" ) )
        :Append( ui.NewHSeparator() )
        :Append( ui.NewDatePicker() )
        :Append( ui.NewTimePicker() )
        :Append( ui.NewDateTimePicker() )
        :Append( ui.NewFontButton() )
        :Append( ui.NewColorButton() )
        :Append( ui.NewButton( "Button 2" ):OnClicked( function() ui.MsgBox( window, "Button 2", "clicked" ) end ) )

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

spinbox:OnChanged( function()
	slider:SetValue( spinbox:Value() )
	progress:SetValue( spinbox:Value() )
end )

slider:OnChanged( function()
	spinbox:SetValue( slider:Value() )
	progress:SetValue( slider:Value() )
end )

local group = ui.NewGroup( "Lists" )
group:SetMargined( true )
inner2:Append( group )

local inner = ui.NewVBox()
inner:SetPadded( true )
group:SetChild( inner )

inner:Append( ui.NewCombobox():Append( "Combobox Item 1" ):Append( "Combobox Item 2" ):Append( "Combobox Item 3" ) )
inner:Append( ui.NewCombobox( true ):Append( "Editable Item 1" ):Append( "Editable Item 2" ):Append( "Editable Item 3" ) )
inner:Append( ui.NewRadioButtons():Append( "Radio Button 1" ):Append( "Radio Button 2" ):Append( "Radio Button 3" ) )

local tab = ui.NewTab()
tab:Append( "Page 1", ui.NewHBox() )
tab:Append( "Page 2", ui.NewHBox() )
tab:Append( "Page 3", ui.NewHBox() )
inner2:Append( tab, true )


window:Show()
ui.Main()
