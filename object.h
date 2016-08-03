#ifndef OBJECT_H
#define OBJECT_H

#include <lua.h>
#include <lauxlib.h>

// simplifies getters/setters
#define luaL_checkboolean( L, i ) lua_toboolean( L, i )
typedef int integer;
typedef int boolean;
typedef double number;

// match against window controls (not menus)
#define uiControlSignature 0x5A5A5A5A

// menus and items are not controls
#define uiMenuSignature 0x54665434
#define uiMenuItemSignature 0x295629A5

// private header?
#include <common/controlsigs.h>

// TODO figure out how to free text in generic getters


void* object_create( lua_State* L, void* object, int signature, ... );
int object_copy( lua_State* L, void* object );
void* check_object( lua_State* L, int idx, int signature );
int object_gc( lua_State* L );



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

#define DECLARE_SETTER_2( typename, action, typea, typeb ) \
	int l_ ## typename ## action( lua_State* L ) { \
		typename ## action( (typename*) check_object( L, 1, typename ## Signature ), luaL_check ## typea ( L, 2 ), luaL_check ## typeb ( L, 3 ) ); \
		lua_pushvalue( L, 1 ); \
		return 1; \
	}

#define DECLARE_GETTER_2( typename, action, typea, typeb ) \
	int l_ ## typename ## action( lua_State* L ) { \
		typea a; \
		typeb b; \
		typename ## action ( (typename*) check_object( L, 1, typename ## Signature ), &a, &b ); \
		lua_push ## typea ( L, a ); \
		lua_push ## typeb ( L, b ); \
		return 1; \
	}

#define DECLARE_ACTION( typename, action ) \
    int l_ ## typename ## action( lua_State* L ) { \
        typename ## action ( (typename*) check_object( L, 1, typename ## Signature ) ); \
        lua_pushvalue( L, 1 ); \
        return 1; \
    }

#endif /* OBJECT_H */
