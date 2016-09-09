#ifndef OBJECT_H
#define OBJECT_H

#include <lua.h>
#include <lauxlib.h>

// simplifies getters/setters
#define luaL_checkboolean( L, i ) lua_toboolean( L, i )
typedef int integer;
typedef int boolean;
typedef double number;

// TODO figure out how to free text in generic getters


void* object_create( lua_State* L, void* object, int signature, ... );
int object_copy( lua_State* L, void* object );
void* get_object( lua_State* L, int idx, int* signature );
void* get_object_unsafe( lua_State* L, int idx );
void* check_object( lua_State* L, int idx, int signature );
int is_object( lua_State* L, int idx, int signature );
int object_gc( lua_State* L );

// get object userdata from registry, with a meta function
// suitable for use with lua_call()
void object_retreive_with_function( lua_State* L, void* obj, char const* metafunction );



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
