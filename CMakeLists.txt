cmake_minimum_required( VERSION 2.8.8 )
project( libui-lua )

SET( LUA_VERSION 5.1 )

SET( LUA_DIR "" )
if( WIN32 )
	SET( LUA_DIR "../lua-5.3.3" )
endif( WIN32 )

find_package( PkgConfig QUIET )
if( PkgConfig_FOUND )
	pkg_check_modules( PC_LUA lua-${LUA_VERSION} )
endif( PkgConfig_FOUND )

if( PC_LUA_FOUND )
	set( LUA_DEFINITIONS ${PC_LUA_CFLAGS_OTHER})
	set( LUA_INCLUDES ${PC_LUA_INCLUDE_DIRS} )
	set( LUA_LIBRARY ${PC_LUA_LIBRARIES} )
else( PC_LUA_FOUND )
	find_path( LUA_INCLUDES lua.h HINTS ${LUA_DIR} PATH_SUFFIXES src )
	find_library( LUA_LIBRARY NAMES lua lua52 lua53 liblua liblua52 liblua53 HINTS ${LUA_DIR} PATH_SUFFIXES build-vs/src )
endif( PC_LUA_FOUND )

find_path( LIBUI_INCLUDES ui.h HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../libui )
find_library( LIBUI_LIBRARY NAMES libui ui HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../libui PATH_SUFFIXES out build/out build-vs/out ) 

set( libui-lua-sources callback.c control-common.c controls.c image.c libui-lua.c menu.c object.c area.c draw.c lua-compat.c )

include_directories( ${LUA_INCLUDES} ${LIBUI_INCLUDES} )

add_library( core SHARED ${libui-lua-sources} )
set_target_properties( core PROPERTIES PREFIX "" )
target_link_libraries( core ${LIBUI_LIBRARY} ${LUA_LIBRARY} )

set_target_properties( core PROPERTIES COMPILE_FLAGS "-fvisibility=hidden" )
