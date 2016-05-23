#include <lua.h>
#include <lauxlib.h>

#include <windows.h>

#include <ui.h>
#include <ui_windows.h>


// contained in platform specific file (libui-windows-lua.c or similar)
int open_platform_specific( lua_State* L )
{
	return 0;
}
