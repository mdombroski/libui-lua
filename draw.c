#include "draw.h"

#include "object.h"
#include "callback.h"
#include "control-common.h"
#include "image.h"

#include <lua.h>
#include <lauxlib.h>

#include <ui.h>

#include <stdlib.h>
#include <string.h>

// for the meta-object type verification (check_object)
#define uiDrawContextSignature		0xabe45355
#define uiDrawPathSignature			0x99334fe4
#define uiDrawBrushSignature		0x9673443a
#define uiDrawTextFontSignature		0xab233454
#define uiDrawTextLayoutSignature	0xcd345326
#define uiDrawMatrixSignature		0xaef44356


#define DRAW_SETFIELD_NUMBER( L, n, name ) \
lua_pushnumber( L, n ); \
lua_setfield( L, -2, name );

// this is a GCC language extension!
// https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html
#define DRAW_TABLE_NUMBER_OR_DEFAULT( L, idx, name, def ) ({ \
double v = def; \
lua_getfield( L, idx, name ); \
if( lua_isnumber( L, -1 ) ) \
	v = lua_tonumber( L, -1 ); \
lua_pop( L, 1 ); v; })

#define check_context( L, idx ) ( (uiDrawContext*) check_object( L, idx, uiDrawContextSignature ) )
#define check_path( L, idx )    ( (uiDrawPath*) check_object( L, idx, uiDrawPathSignature ) )
#define check_font( L, idx )    ( (uiDrawTextFont*) check_object( L, idx, uiDrawTextFontSignature ) )
#define check_layout( L, idx )  ( (uiDrawTextLayout*) check_object( L, idx, uiDrawTextLayoutSignature ) )
#define check_brush( L, idx )   ( (uiDrawBrush*) check_object( L, idx, uiDrawBrushSignature ) )
#define check_matrix( L, idx )  ( (uiDrawMatrix*) check_object( L, idx, uiDrawMatrixSignature ) )



static int l_draw_stroke( lua_State* L )
{
	struct uiDrawStrokeParams params;
	params.Cap = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 4, "cap", uiDrawLineCapFlat );
	params.Join = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 4, "join", uiDrawLineJoinMiter );
	params.Thickness = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 4, "thickness", 1 );
	params.MiterLimit = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 4, "miterlimit", 1 );
	// TODO: support dashes
	params.NumDashes = 0;
	params.DashPhase = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 4, "dashphase", 0 );

	uiDrawStroke( check_context( L, 1 ), check_path( L, 2 ), check_brush( L, 3 ), &params );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_draw_fill( lua_State* L )
{
	uiDrawFill( check_context( L, 1 ), check_path( L, 2 ), check_brush( L, 3 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_draw_transform( lua_State* L )
{
	uiDrawTransform( check_context( L, 1 ), check_matrix( L, 2 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_draw_clip( lua_State* L )
{
	uiDrawClip( check_context( L, 1 ), check_path( L, 2 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_draw_save( lua_State* L )
{
	uiDrawSave( check_context( L, 1 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_draw_restore( lua_State* L )
{
	uiDrawRestore( check_context( L, 1 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_draw_text( lua_State* L )
{
	uiDrawText( check_context( L, 1 ), luaL_checknumber( L, 3 ), luaL_checknumber( L, 4 ), check_layout( L, 2 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static luaL_Reg context_functions[] =
{
	{ "Stroke", l_draw_stroke },
	{ "Fill", l_draw_fill },
	{ "Transform", l_draw_transform },
	{ "Clip", l_draw_clip },
	{ "Save", l_draw_save },
	{ "Restore", l_draw_restore },
	{ "Text", l_draw_text },
	{ 0, 0 }
};

int draw_new_context( lua_State* L, uiDrawContext* ctx )
{
	object_create( L, ctx, uiDrawContextSignature, context_functions, 0 );
	return 1;
}



static int l_path_new_figure( lua_State* L )
{
	uiDrawPathNewFigure(
		(uiDrawPath*) check_object( L, 1, uiDrawPathSignature ), 
		luaL_checknumber( L, 2 ),
		luaL_checknumber( L, 3 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_path_new_figure_arc( lua_State* L )
{
	uiDrawPathNewFigureWithArc(
		(uiDrawPath*) check_object( L, 1, uiDrawPathSignature ), 
		luaL_checknumber( L, 2 ),
		luaL_checknumber( L, 3 ), 
		luaL_checknumber( L, 4 ),
		luaL_checknumber( L, 5 ), 
		luaL_checknumber( L, 6 ),
		luaL_checknumber( L, 7 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_path_lineto( lua_State* L )
{
	uiDrawPathLineTo(
		(uiDrawPath*) check_object( L, 1, uiDrawPathSignature ), 
		luaL_checknumber( L, 2 ),
		luaL_checknumber( L, 3 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_path_arcto( lua_State* L )
{
	uiDrawPathArcTo(
		(uiDrawPath*) check_object( L, 1, uiDrawPathSignature ), 
		luaL_checknumber( L, 2 ),
		luaL_checknumber( L, 3 ), 
		luaL_checknumber( L, 4 ),
		luaL_checknumber( L, 5 ), 
		luaL_checknumber( L, 6 ),
		luaL_checknumber( L, 7 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_path_bezierto( lua_State* L )
{
	uiDrawPathBezierTo(
		(uiDrawPath*) check_object( L, 1, uiDrawPathSignature ), 
		luaL_checknumber( L, 2 ),
		luaL_checknumber( L, 3 ), 
		luaL_checknumber( L, 4 ),
		luaL_checknumber( L, 5 ), 
		luaL_checknumber( L, 6 ),
		luaL_checknumber( L, 7 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_path_add_rectangle( lua_State* L )
{
	uiDrawPathAddRectangle(
		(uiDrawPath*) check_object( L, 1, uiDrawPathSignature ), 
	   luaL_checknumber( L, 2 ),
	   luaL_checknumber( L, 3 ), 
	   luaL_checknumber( L, 4 ),
	   luaL_checknumber( L, 5 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

DECLARE_ACTION( uiDrawPath, CloseFigure )
DECLARE_ACTION( uiDrawPath, End )

static int l_path_gc( lua_State* L )
{
	uiDrawFreePath( (uiDrawPath*) check_object( L, 1, uiDrawPathSignature ) );
	return 0;
}


static luaL_Reg path_functions[] =
{
	{ "NewFigure", l_path_new_figure },
	{ "NewFigureWithArc", l_path_new_figure_arc },
	{ "LineTo", l_path_lineto },
	{ "ArcTo", l_path_arcto },
	{ "BezierTo", l_path_bezierto },
	{ "AddRectangle", l_path_add_rectangle },
	
	{ "CloseFigure", l_uiDrawPathCloseFigure },
	{ "End", l_uiDrawPathEnd },

	{ "__gc", l_path_gc },
	{ 0, 0 }
};

static int new_path( lua_State* L )
{
	uiDrawPath* path = uiDrawNewPath( luaL_checknumber( L, 1 ) );
	object_create( L, path, uiDrawPathSignature, path_functions, 0 );
	return 1;
}



static int l_brush_rgba( lua_State* L )
{
	uiDrawBrush* brush = check_brush( L, 1 );
	if( brush->Stops )
	{
		free( brush->Stops );
	}
	memset( brush, 0, sizeof(*brush) );

	brush->Type = uiDrawBrushTypeSolid;
	brush->R = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 2, "r", 0 );
	brush->G = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 2, "g", 0 );
	brush->B = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 2, "b", 0 );
	brush->A = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 2, "a", 1.0 );

	lua_pushvalue( L, 1 );
	return 1;
}

static void brush_gradient_common( lua_State* L, uiDrawBrush* brush )
{
	// number of stops
	lua_len( L, 3 );
	int n = lua_tonumber( L, -1 );
	lua_pop( L, 1 );
	if( n < 1 )
	{
		return;
	}
	
	// get first coordinate (X0,Y0)
	lua_pushnumber( L, 1 );
	lua_gettable( L, 2 );
	brush->X0 = DRAW_TABLE_NUMBER_OR_DEFAULT( L, -1, "x", 0 );
	brush->Y0 = DRAW_TABLE_NUMBER_OR_DEFAULT( L, -1, "y", 0 );
	lua_pop( L, 1 );
	
	// get second coordinate (X1,Y1)
	lua_pushnumber( L, 2 );
	lua_gettable( L, 2 );
	brush->X1 = DRAW_TABLE_NUMBER_OR_DEFAULT( L, -1, "x", 0 );
	brush->Y1 = DRAW_TABLE_NUMBER_OR_DEFAULT( L, -1, "y", 0 );
	lua_pop( L, 1 );

	// For radial only: Outer Radius
	brush->OuterRadius = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 2, "OuterRadius", 0 );

	brush->Stops = malloc( sizeof(uiDrawBrushGradientStop) * n );
	memset( brush->Stops, 0, sizeof(uiDrawBrushGradientStop) * n );
	brush->NumStops = n;

	for( int i = 0; i < n; ++i )
	{
		// get table for gradient stop
		lua_pushnumber( L, i + 1 );
		lua_gettable( L, 3 );
		
		brush->Stops[ i ].Pos = DRAW_TABLE_NUMBER_OR_DEFAULT( L, -1, "pos", 0 );
		brush->Stops[ i ].R = DRAW_TABLE_NUMBER_OR_DEFAULT( L, -1, "r", 0 );
		brush->Stops[ i ].G = DRAW_TABLE_NUMBER_OR_DEFAULT( L, -1, "g", 0 );
		brush->Stops[ i ].B = DRAW_TABLE_NUMBER_OR_DEFAULT( L, -1, "b", 0 );
		brush->Stops[ i ].A = DRAW_TABLE_NUMBER_OR_DEFAULT( L, -1, "a", 1.0 );

		lua_pop( L, 1 );
	}
}

static int l_brush_linear( lua_State* L )
{
	uiDrawBrush* brush = check_brush( L, 1 );
	if( brush->Stops )
	{
		free( brush->Stops );
	}
	memset( brush, 0, sizeof(*brush) );

	if( ! lua_istable( L, 2 ) || ! lua_istable( L, 3 ) )
	{
		luaL_error( L, "incorrect arguments (should be tables)" );
	}

	brush->Type = uiDrawBrushTypeLinearGradient;

	brush_gradient_common( L, brush );

	lua_pushvalue( L, 1 );
	return 1;
}

static int l_brush_radial( lua_State* L )
{
	uiDrawBrush* brush = check_brush( L, 1 );
	if( brush->Stops )
	{
		free( brush->Stops );
	}
	memset( brush, 0, sizeof(*brush) );

	brush->Type = uiDrawBrushTypeRadialGradient;

	brush_gradient_common( L, brush );

	lua_pushvalue( L, 1 );
	return 1;
}

static int l_brush_gc( lua_State* L )
{
	uiDrawBrush* brush = check_brush( L, 1 );
	if( brush->Stops )
	{
		free( brush->Stops );
	}
	free( brush );
	return 0;
}

static luaL_Reg brush_functions[] =
{
	{ "rgba", l_brush_rgba },
	{ "linear", l_brush_linear },
	{ "radial", l_brush_radial },
	{ "__gc", l_brush_gc },
	{ 0, 0 }
};

static int new_brush( lua_State* L )
{
	uiDrawBrush* brush = malloc( sizeof(uiDrawBrush) );
	memset( brush, 0, sizeof(*brush) );

	object_create( L, brush, uiDrawBrushSignature, brush_functions, 0 );
	return 1;
}



// this isnt fully implemented upsteam in libui!
// don't trust it!
static int l_font_describe( lua_State* L )
{
	uiDrawTextFontDescriptor desc;
	memset( &desc, 0, sizeof(desc) );

	uiDrawTextFont* font = check_object( L, 1, uiDrawTextFontSignature );
	uiDrawTextFontDescribe( font, &desc );

	lua_newtable( L );
	if( desc.Family )
	{
		lua_pushstring( L, desc.Family );
		lua_setfield( L, -2, "family" );
	}
	lua_pushnumber( L, desc.Size );
	lua_setfield( L, -2, "size" );
	lua_pushnumber( L, desc.Weight );
	lua_setfield( L, -2, "weight" );
	lua_pushnumber( L, desc.Italic );
	lua_setfield( L, -2, "style" );
	lua_pushnumber( L, desc.Stretch );
	lua_setfield( L, -2, "stretch" );
	return 1;
}

static int l_font_metrics( lua_State* L )
{
	uiDrawTextFontMetrics metrics;
	uiDrawTextFont* font = check_object( L, 1, uiDrawTextFontSignature );
	uiDrawTextFontGetMetrics( font, &metrics );

	lua_newtable( L );
	lua_pushnumber( L, metrics.Ascent );
	lua_setfield( L, -2, "ascent" );
	lua_pushnumber( L, metrics.Descent );
	lua_setfield( L, -2, "descent" );
	lua_pushnumber( L, metrics.Leading );
	lua_setfield( L, -2, "leading" );
	lua_newtable( L );
	lua_pushnumber( L, metrics.UnderlinePos );
	lua_setfield( L, -2, "position" );
	lua_pushnumber( L, metrics.UnderlineThickness );
	lua_setfield( L, -2, "thickness" );
	lua_setfield( L, -2, "underline" );
	return 1;
}

static int l_font_gc( lua_State* L )
{
	uiDrawFreeTextFont( (uiDrawTextFont*) check_object( L, 1, uiDrawTextFontSignature ) ); 
	return 0;
}

static luaL_Reg font_functions[] =
{
	{ "Describe", l_font_describe },
	{ "Metrics", l_font_metrics },
	{ "__gc", l_font_gc },
	{ 0, 0 }
};

int draw_new_font( lua_State* L, uiDrawTextFont* font )
{
	if( font )
		object_create( L, font, uiDrawTextFontSignature, font_functions, 0 );
	else
		lua_pushnil( L );
	return 1;
}

static int new_font( lua_State* L )
{
	uiDrawTextFontDescriptor descriptor;
	
	descriptor.Size = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 1, "size", 10 );
	descriptor.Weight = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 1, "weight", uiDrawTextWeightNormal );
	descriptor.Italic = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 1, "style", uiDrawTextItalicNormal );
	descriptor.Stretch = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 1, "stretch", uiDrawTextStretchNormal );

	descriptor.Family = "sans-serif";
	lua_getfield( L, 1, "family" );
	if( lua_isstring( L, -1 ) )
		descriptor.Family = lua_tostring( L, -1 );

	uiDrawTextFont* font = uiDrawLoadClosestFont( &descriptor );

	// font family name is now invalid
	lua_pop( L, 1 );

	return draw_new_font( L, font );
}


static int l_layout_set_width( lua_State* L )
{
	uiDrawTextLayoutSetWidth( (uiDrawTextLayout*) check_object( L, 1, uiDrawTextLayoutSignature ), luaL_checknumber( L, 2 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_layout_extents( lua_State* L )
{
	double width, height;
	uiDrawTextLayoutExtents( (uiDrawTextLayout*) check_object( L, 1, uiDrawTextLayoutSignature ), &width, &height );
	lua_newtable( L );
	lua_pushnumber( L, width );
	lua_setfield( L, -2, "width" );
	lua_pushnumber( L, height );
	lua_setfield( L, -2, "height" );
	return 1;
}

static int l_layout_set_colour( lua_State* L )
{
	uiDrawTextLayout* layout = check_object( L, 1, uiDrawTextLayoutSignature );
	int startchar = luaL_checknumber( L, 2 );
	int endchar = luaL_checknumber( L, 3 );

	double r = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 4, "r", 0 );
	double g = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 4, "g", 0 );
	double b = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 4, "b", 0 );
	double a = DRAW_TABLE_NUMBER_OR_DEFAULT( L, 4, "a", 1.0 );

	uiDrawTextLayoutSetColor( layout, startchar, endchar, r, g, b, a );

	lua_pushvalue( L, 1 );
	return 1;
}

static int l_layout_gc( lua_State* L )
{
	uiDrawFreeTextLayout( (uiDrawTextLayout*) check_object( L, 1, uiDrawTextLayoutSignature ) );
	return 0;
}

static luaL_Reg layout_functions[] =
{
	{ "SetWidth", l_layout_set_width },
	{ "Extents", l_layout_extents },
	{ "SetColour", l_layout_set_colour },
	{ "SetColor", l_layout_set_colour },
	{ "__gc", l_layout_gc },
	{ 0, 0 }
};

static int new_layout( lua_State* L )
{
	char const* text = luaL_checkstring( L, 1 );
	uiDrawTextFont* font = check_object( L, 2, uiDrawTextFontSignature );
	double width = luaL_checknumber( L, 3 );
	
	uiDrawTextLayout* layout = uiDrawNewTextLayout( text, font, width );
	object_create( L, layout, uiDrawTextLayoutSignature, layout_functions, 0 );
	return 1;
}



DECLARE_SETTER_2( uiDrawMatrix, Translate, number, number )
DECLARE_GETTER( uiDrawMatrix, Invertible, boolean )
DECLARE_ACTION( uiDrawMatrix, Invert )

static int l_matrix_identity( lua_State* L );

static int l_matrix_scale( lua_State* L )
{
	uiDrawMatrixScale( check_matrix( L, 1 ), luaL_checknumber( L, 2 ), luaL_checknumber( L, 3 ), luaL_checknumber( L, 4 ), luaL_checknumber( L, 5 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_matrix_rotate( lua_State* L )
{
	uiDrawMatrixRotate( check_matrix( L, 1 ), luaL_checknumber( L, 2 ), luaL_checknumber( L, 3 ), luaL_checknumber( L, 4 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_matrix_skew( lua_State* L )
{
	uiDrawMatrixSkew( check_matrix( L, 1 ), luaL_checknumber( L, 2 ), luaL_checknumber( L, 3 ), luaL_checknumber( L, 4 ), luaL_checknumber( L, 5 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_matrix_multiply( lua_State* L )
{
	uiDrawMatrixMultiply( check_matrix( L, 1 ), check_matrix( L, 2 ) );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_matrix_transform_point( lua_State* L )
{
	double x = luaL_checknumber( L, 2 );
	double y = luaL_checknumber( L, 3 );
	uiDrawMatrixTransformPoint( check_matrix( L, 1 ), &x, &y );
	lua_pushnumber( L, x );
	lua_pushnumber( L, y );
	return 2;
}

static int l_matrix_transform_size( lua_State* L )
{
	double x = luaL_checknumber( L, 2 );
	double y = luaL_checknumber( L, 3 );
	uiDrawMatrixTransformSize( check_matrix( L, 1 ), &x, &y );
	lua_pushnumber( L, x );
	lua_pushnumber( L, y );
	return 2;
}

static int l_matrix_get( lua_State* L )
{
	uiDrawMatrix* m = check_matrix( L, 1 );
	lua_newtable( L );
	lua_pushnumber( L, 1 );
	lua_newtable( L );
	lua_pushnumber( L, 1 );
	lua_pushnumber( L, m->M11 );
	lua_settable( L, -3 );
	lua_pushnumber( L, 2 );
	lua_pushnumber( L, m->M12 );
	lua_settable( L, -3 );
	lua_settable( L, -3 );
	lua_pushnumber( L, 2 );
	lua_newtable( L );
	lua_pushnumber( L, 1 );
	lua_pushnumber( L, m->M21 );
	lua_settable( L, -3 );
	lua_pushnumber( L, 2 );
	lua_pushnumber( L, m->M22 );
	lua_settable( L, -3 );
	lua_settable( L, -3 );
	lua_pushnumber( L, 3 );
	lua_newtable( L );
	lua_pushnumber( L, 1 );
	lua_pushnumber( L, m->M31 );
	lua_settable( L, -3 );
	lua_pushnumber( L, 2 );
	lua_pushnumber( L, m->M32 );
	lua_settable( L, -3 );
	lua_settable( L, -3 );
	return 1;
}

static void matrix_set_helper( lua_State* L, uiDrawMatrix* m, int idx )
{
	// get row 0
	lua_pushnumber( L, 1 );
	lua_gettable( L, idx);
	lua_pushnumber( L, 1 );
	lua_gettable( L, -2 );
	lua_pushnumber( L, 2 );
	lua_gettable( L, -3 );
	m->M11 = lua_tonumber( L, -2 );
	m->M12 = lua_tonumber( L, -1 );
	lua_pop( L, 3 );
	// get row 1
	lua_pushnumber( L, 2 );
	lua_gettable( L, idx );
	lua_pushnumber( L, 1 );
	lua_gettable( L, -2 );
	lua_pushnumber( L, 2 );
	lua_gettable( L, -3 );
	m->M21 = lua_tonumber( L, -2 );
	m->M22 = lua_tonumber( L, -1 );
	lua_pop( L, 3 );
	// get row 2
	lua_pushnumber( L, 3 );
	lua_gettable( L, idx );
	lua_pushnumber( L, 1 );
	lua_gettable( L, -2 );
	lua_pushnumber( L, 2 );
	lua_gettable( L, -3 );
	m->M31 = lua_tonumber( L, -2 );
	m->M32 = lua_tonumber( L, -1 );
	lua_pop( L, 3 );
}

static int l_matrix_set( lua_State* L )
{
	matrix_set_helper( L, check_matrix( L, 1 ), 2 );
	lua_pushvalue( L, 1 );
	return 1;
}

static int l_matrix_tostring( lua_State* L )
{
	uiDrawMatrix* m = check_matrix( L, 1 );
	lua_pushfstring( L, "((%f,%f),(%f,%f),(%f,%f))", m->M11, m->M12, m->M21, m->M22, m->M31, m->M32 );
	return 1;
}

static int l_matrix_gc( lua_State* L )
{
	uiDrawMatrix* m = check_matrix( L, 1 );
	free( m );
	return 0;
}

static luaL_Reg matrix_functions[] =
{
	{ "Translate", l_uiDrawMatrixTranslate },
	{ "Invertible", l_uiDrawMatrixInvertible },
	{ "Invert", l_uiDrawMatrixInvert },

	{ "Identity", l_matrix_identity },
	{ "Scale", l_matrix_scale },
	{ "Rotate", l_matrix_rotate },
	{ "Skew", l_matrix_skew },
	{ "Multiply", l_matrix_multiply },
	{ "TransformPoint", l_matrix_transform_point },
	{ "TransformSize", l_matrix_transform_size },

	{ "Get", l_matrix_get },
	{ "Set", l_matrix_set },

	{ "__tostring", l_matrix_tostring },
	{ "__gc", l_matrix_gc },

	{ 0, 0 }
};

static int l_matrix_identity( lua_State* L )
{
	if( is_object( L, 1, uiDrawMatrixSignature ) )
	{
		uiDrawMatrixSetIdentity( (uiDrawMatrix*) get_object_unsafe( L, 1 ) );
		lua_pushvalue( L, 1 );
	}
	else
	{
		uiDrawMatrix* m = malloc( sizeof(uiDrawMatrix) );
		uiDrawMatrixSetIdentity( m );
		
		object_create( L, m, uiDrawMatrixSignature, matrix_functions, 0 );
	}
	
	return 1;
}

static int new_matrix( lua_State* L )
{
	uiDrawMatrix* m = malloc( sizeof(uiDrawMatrix) );

	// copy
	if( is_object( L, 1, uiDrawMatrixSignature ) )
	{
		memcpy( m, get_object_unsafe( L, 1 ), sizeof(*m) );
	}
	// supplied table with states
	else
	if( lua_istable( L, 1 ) )
	{
		matrix_set_helper( L, m, 1 );
	}
	// identity
	else
	{
		uiDrawMatrixSetIdentity( m );
	}

	object_create( L, m, uiDrawMatrixSignature, matrix_functions, 0 );
	return 1;
}

static int get_font_families( lua_State* L )
{
	lua_newtable( L );

	uiDrawFontFamilies* families = uiDrawListFontFamilies();
	int n = uiDrawFontFamiliesNumFamilies( families );

	for( int i = 0; i < n; ++i )
	{
		lua_pushnumber( L, i + 1 );
		lua_pushstring( L, uiDrawFontFamiliesFamily( families, i ) );
		lua_settable( L, -3 );
	}

	uiDrawFreeFontFamilies( families );

	return 1;
}

static luaL_Reg draw_functions[] =
{
	{ "Path", new_path },
	{ "Brush", new_brush },
	{ "TextLayout", new_layout },
	{ 0, 0 }
};

void draw_init( lua_State* L )
{
	lua_newtable( L ); // Draw
	
	luaL_setfuncs( L, draw_functions, 0 );
	
	lua_newtable( L ); // Draw.Line
	lua_newtable( L ); // Draw.Line.Cap
	DRAW_SETFIELD_NUMBER( L, uiDrawLineCapFlat, "Flat" );
	DRAW_SETFIELD_NUMBER( L, uiDrawLineCapRound, "Round" );
	DRAW_SETFIELD_NUMBER( L, uiDrawLineCapSquare, "Square" );
	lua_setfield( L, -2, "Cap" );
	
	lua_newtable( L ); // Draw.Line.Join
	DRAW_SETFIELD_NUMBER( L, uiDrawLineJoinMiter, "Miter" );
	DRAW_SETFIELD_NUMBER( L, uiDrawLineJoinRound, "Round" );
	DRAW_SETFIELD_NUMBER( L, uiDrawLineJoinBevel, "Bevel" );
	lua_setfield( L, -2, "Join" );
	lua_setfield( L, -2, "Line" );

	lua_newtable( L ); // Draw.FillMode
	DRAW_SETFIELD_NUMBER( L, uiDrawFillModeWinding, "Winding" );
	DRAW_SETFIELD_NUMBER( L, uiDrawFillModeAlternate, "Alternate" );
	lua_setfield( L, -2, "FillMode" );

	lua_pushcfunction( L, new_font ); // Draw.Font
	lua_newtable( L );
	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );
	lua_pushcfunction( L, get_font_families );
	lua_setfield( L, -2, "Families" );
	lua_newtable( L ); // Draw.Font.Weight
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightThin, "Thin" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightUltraLight, "UltraLight" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightLight, "Light" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightBook, "Book" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightNormal, "Normal" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightMedium, "Medium" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightSemiBold, "SemiBold" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightBold, "Bold" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightUtraBold, "UltraBold" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightHeavy, "Heavy" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextWeightUltraHeavy, "UltraHeavy" );
	lua_setfield( L, -2, "Weight" );
	
	lua_newtable( L ); // Draw.Font.Style
	DRAW_SETFIELD_NUMBER( L, uiDrawTextItalicNormal, "Normal" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextItalicOblique, "Oblique" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextItalicItalic, "Italic" );
	lua_setfield( L, -2, "Style" );
	
	lua_newtable( L ); // Draw.Font.Stretch
	DRAW_SETFIELD_NUMBER( L, uiDrawTextStretchUltraCondensed, "UltraCondensed" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextStretchExtraCondensed, "ExtraCondensed" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextStretchCondensed, "Condensed" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextStretchSemiCondensed, "SemiCondensed" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextStretchNormal, "Normal" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextStretchSemiExpanded, "SemiExpanded" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextStretchExpanded, "Expanded" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextStretchExtraExpanded, "ExtraExpanded" );
	DRAW_SETFIELD_NUMBER( L, uiDrawTextStretchUltraExpanded, "UltraExpanded" );
	lua_setfield( L, -2, "Stretch" );
	lua_setmetatable( L, -2 );
	lua_setfield( L, -2, "Font" );

	lua_pushcfunction( L, new_matrix );
	lua_newtable( L );
	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );
	lua_pushcfunction( L, l_matrix_identity );
	lua_setfield( L, -2, "Identity" );
	lua_setmetatable( L, -2 );
	lua_setfield( L, -2, "Matrix" );

	lua_setfield( L, -2, "Draw" );
}
