#ifndef IMAGE_H
#define IMAGE_H

#include <ui.h>
#include <lauxlib.h>

extern luaL_Reg image_functions[];

int is_image( lua_State* L, int index );
uiImage* check_image( lua_State* L, int index );


#endif // IMAGE_H

