#ifndef LUA_COMPAT_H
#define LUA_COMPAT_H

#include <lua.h>
#include <lauxlib.h>

// implementing support for old Lua versions

#if LUA_VERSION_NUM < 501
#error Lua < 5.1 not supported
#endif

// Functions not available in Lua < 5.3
#if LUA_VERSION_NUM < 503
int lua_isinteger( lua_State* L, int idx );
void lua_rotate( lua_State* L, int idx, int n );
#endif

// Functions not available in Lua < 5.2
#if LUA_VERSION_NUM < 502
int lua_absindex( lua_State *L, int i );
void lua_len( lua_State* L, int i );
void luaL_setfuncs( lua_State* L, luaL_Reg* l, int nup );

#define luaL_newlib(L,l)  \
	(lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1), luaL_setfuncs(L,l,0))
#endif

#endif // LUA_COMPAT_H
