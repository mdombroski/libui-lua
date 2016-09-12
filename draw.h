#ifndef DRAW_H
#define DRAW_H

#include <lauxlib.h>
#include <ui.h>

void draw_init( lua_State* L );

int draw_new_context( lua_State* L, uiDrawContext* ctx );
int draw_new_font( lua_State* L, uiDrawTextFont* font );

#endif /* DRAW_H */
