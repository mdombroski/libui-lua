#include "image.h"
#include "object.h"

#include <stdlib.h>
#include <lauxlib.h>

#define uiImageSignature 0x45363456


static luaL_Reg image_members[];


int is_image( lua_State* L, int index )
{
	return is_object( L, index, uiImageSignature );
}


uiImage* check_image( lua_State* L, int index )
{
	return (uiImage*) check_object( L, index, uiImageSignature );
}


static int l_Load( lua_State* L )
{
	uiImage* i = (uiImage*) check_object( L, 1, uiImageSignature );

	if( uiImageLoad( i, luaL_checkstring( L, 2 ) ) )
	{
		lua_pushvalue( L, 1 );
		return 1;
	}

	luaL_error( L, "libui: image could not be loaded." );
	return 0;
}

static int l_Valid( lua_State* L )
{
	uiImage *i = (uiImage*) check_object( L, 1, uiImageSignature );
	lua_pushboolean( L, uiImageValid( i ) );
	return 1;
}

static int l_Size( lua_State* L )
{
	uiImage *i = (uiImage*) check_object( L, 1, uiImageSignature );
	int width = -1, height = -1;
	uiImageSize( i, &width, &height );
	lua_pushinteger( L, width );
	lua_pushinteger( L, height );
	return 2;
}

static int l_Resize( lua_State* L )
{
	uiImage *i = (uiImage*) check_object( L, 1, uiImageSignature );
	uiImage *n = uiImageResize( i, luaL_checkinteger( L, 2 ), luaL_checkinteger( L, 3 ) );
	object_create( L, n, uiImageSignature, image_members, 0 );
	return 1;
}

static int l_Destroy( lua_State* L )
{
	printf( "gc image\n" );
	uiImage *i = (uiImage*) check_object( L, 1, uiImageSignature );
	uiImageDestroy( i );
	return 0;
}


static luaL_Reg image_members[] =
{
	{ "Load", l_Load },
	{ "Valid", l_Valid },
	{ "Size", l_Size },
	{ "Resize", l_Resize },
	{ "__gc", l_Destroy },
	{ 0, 0 }
};

int new_image( lua_State* L )
{
	uiImage* maybecopy = 0;
	if( lua_isuserdata( L, 1 ) )
	{
		int s;
		void* p = get_object( L, 1, &s );
		if( s == uiImageSignature )
		{
			maybecopy = p;
		}
	}

	uiImage* i = uiNewImage( maybecopy );
	
	if( lua_isstring( L, 1 ) )
	{
		uiImageLoad( i, lua_tostring( L, 1 ) );
	}
	
	object_create( L, i, uiImageSignature, image_members, 0 );
	return 1;
}

luaL_Reg image_functions[] =
{
	{ "NewImage", new_image },
	{ 0, 0 }
};


