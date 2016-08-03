#ifndef CALLBACK_H
#define CALLBACK_H

#include <lua.h>
#include "object.h"

enum
{
	callback_ShouldQuit				= 0xDA0,
	callback_OnClosing				= 0xDA1,
	callback_OnClicked				= 0xDA2,
	callback_OnToggled				= 0xDA3,
	callback_OnChanged				= 0xDA4,
	callback_OnSelected				= 0xDA5,
	callback_OnPositionChanged		= 0xDA6,
	callback_OnContentSizeChanged	= 0xDA7,
};


void register_callback( lua_State* L, int idx, void* sender, int callback );
int invoke_callback( lua_State* L, void* id, int callback, int args );

#define DECLARE_CALLBACK_FUNCTION( typename, action ) \
    static void callback_ ## typename ## action( typename *c, void* data ) { \
        invoke_callback( (lua_State*) data, c, callback_ ## action, 0 ); \
    }
#define DECLARE_CALLBACK_REGISTER( typename, signature, action ) \
    static int l_ ## typename ## action( lua_State* L ) { \
        typename* c = (typename*) check_object( L, 1, signature ); \
        register_callback( L, 2, c, callback_ ## action ); \
        if( lua_type( L, 2 ) == LUA_TFUNCTION ) \
            typename ## action( c, callback_ ## typename ## action, L ); \
        lua_pushvalue( L, 1 ); \
        return 1; \
    }
#define DECLARE_CALLBACK( typename, action ) \
    DECLARE_CALLBACK_FUNCTION( typename, action ) \
    DECLARE_CALLBACK_REGISTER( typename, typename ## Signature, action )


#endif /* CALLBACK_H */
