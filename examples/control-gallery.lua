local ui = require "libui.core"



local window = ui.NewWindow( "libui Control Gallery", 640, 480, 1 )


ui.ShouldQuit( function()
	window:Destroy()
end )
--[[
local menu = ui.NewMenu( "File" )
local item = menu:AppendItem( "Open" )
item:OnClicked( function()
	local filename = ui.OpenFile( window )
	if not filename == nil then
		ui.MsgBoxError( window, "No file selected", "Don't be alarmed!" )
	else
		ui.MsgBox( window, "File selected", filename )
	end
end )
local item = menu:AppendItem( "Save" )
item:OnClicked( function()
	local filename = ui.SaveFile( window )
	if not filename == nil then
		ui.MsgBoxError( window, "No file selected", "Don't be alarmed!" )
	else
		ui.MsgBox( window, "File selected (don't worry, it's still there)", filename )
	end
end )
local item = menu:MenuAppendQuitItem()


local menu = ui.NewMenu( "Edit" )
local item = menu:AppendCheckItem( "Checkable Item" )
menu:AppendSeparator()
local item = menu:AppendItem( "Disabled Item" )
item:Disable()
local item = menu:AppendPreferencesItem()

local menu = ui.NewMenu( "Help" )
local item = menu:AppendItem( "Help" )
local item = menu:AppendAboutItem()
]]--

window:SetMargined( true )
window:OnClosing( function()
	window:Destroy()
	ui.Quit()
end )

local box = ui.NewVBox()
box:SetPadded( true )
window:SetChild( box )

local hbox = ui.NewHBox()
hbox:SetPadded( true )
box:Append( hbox, true )

local group = ui.NewGroup( "Basic Controls" )
group:SetMargined( true )
hbox:Append( group )

local inner = ui.NewVBox()
inner:SetPadded( true )
group:SetChild( inner )

inner:Append( ui.NewButton( "Button" ) )
inner:Append( ui.NewCheckbox( "Checkbox" ) )
inner:Append( ui.NewEntry( "Entry" ) )
inner:Append( ui.NewLabel( "Label" ) )
inner:Append( ui.NewHSeparator() )
inner:Append( ui.NewDatePicker() )
inner:Append( ui.NewTimePicker() )
inner:Append( ui.NewDateTimePicker() )
inner:Append( ui.NewFontButton() )
inner:Append( ui.NewColorButton() )

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

local cbox = ui.NewCombobox()
cbox:Append( "Combobox Item 1" )
cbox:Append( "Combobox Item 2" )
cbox:Append( "Combobox Item 3" )
inner:Append( cbox )

local cbox = ui.NewCombobox( true )
cbox:Append( "Editable Item 1" )
cbox:Append( "Editable Item 2" )
cbox:Append( "Editable Item 3" )
inner:Append( cbox )

local rb = ui.NewRadioButtons()
rb:Append( "Radio Button 1" )
rb:Append( "Radio Button 2" )
rb:Append( "Radio Button 3" )
inner:Append( rb )

local tab = ui.NewTab()
tab:Append( "Page 1", ui.NewHBox() )
tab:Append( "Page 2", ui.NewHBox() )
tab:Append( "Page 3", ui.NewHBox() )
inner2:Append( tab, true )


window:Show()
ui.Main()
