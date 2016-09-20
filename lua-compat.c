#include "lua-compat.h"

// implementing support for old Lua versions
// See https://github.com/keplerproject/lua-compat-5.3/blob/master/c-api/compat-5.3.c

// Functions not available in Lua 5.1 or 5.2
#if LUA_VERSION_NUM < 503
int lua_isinteger( lua_State *L, int index )
{
	if( lua_type( L, index ) == LUA_TNUMBER )
	{
		lua_Number n = lua_tonumber( L, index );
		lua_Integer i = lua_tointeger( L, index );
		if( i == n )
			return 1;
	}
	return 0;
}


static void _reverse ( lua_State *L, int a, int b )
{
	for( ; a < b; ++a, --b )
	{
		lua_pushvalue( L, a );
		lua_pushvalue( L, b );
		lua_replace( L, a );
		lua_replace( L, b );
	}
}


void lua_rotate( lua_State *L, int idx, int n )
{
	int n_elems = 0;
	idx = lua_absindex( L, idx );
	n_elems = lua_gettop( L ) - idx + 1;
	if( n < 0 )
	{
		n += n_elems;
	}
	if( n > 0 && n < n_elems )
	{
		luaL_checkstack( L, 2, "not enough stack slots available" );
		n = n_elems - n;
		_reverse( L, idx, idx + n - 1 );
		_reverse( L, idx + n, idx + n_elems - 1 );
		_reverse( L, idx, idx + n_elems - 1 );
	}
}
#endif


// Functions not available in Lua 5.1
#if LUA_VERSION_NUM < 502
int lua_absindex( lua_State *L, int i )
{
	if( i < 0 && i > LUA_REGISTRYINDEX )
		i += lua_gettop( L ) + 1;
	return i;
}


void lua_len( lua_State *L, int i )
{
	switch( lua_type( L, i ) )
	{
		case LUA_TSTRING:
			lua_pushnumber( L, (lua_Integer) lua_objlen( L, i ) );
			break;
		case LUA_TTABLE:
			if( !luaL_callmeta(L, i, "__len" ) )
				lua_pushnumber( L, (lua_Integer) lua_objlen( L, i ) );
			break;
		case LUA_TUSERDATA:
			if( luaL_callmeta( L, i, "__len" ) )
				break;
			/* maybe fall through */
			default:
				luaL_error( L, "attempt to get length of a %s value",
						   lua_typename( L, lua_type( L, i ) ) );
	}
}


// copied from Lua 5.3 source code
void luaL_setfuncs( lua_State* L, luaL_Reg* l, int nup )
{
	luaL_checkstack(L, nup, "too many upvalues");
	for (; l->name != NULL; l++) {  /* fill the table with given functions */
		int i;
		for (i = 0; i < nup; i++)  /* copy upvalues to the top */
			lua_pushvalue(L, -nup);
		lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
		lua_setfield(L, -(nup + 2), l->name);
	}
	lua_pop(L, nup);  /* remove upvalues */
}
#endif

