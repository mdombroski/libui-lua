package = "libui-lua"
version = "git-0"

description = {
	summary = "Lua binding for libui";
	detailed = [[Lua binding for libui: https://github.com/mdombroski/libui-lua]];
	homepage = "https://github.com/mdombroski/libui-lua";
}

source = {
	url = "git+https://github.com/mdombroski/libui-lua.git";
}

dependencies = {
	"lua >= 5.1, < 5.4"
}

external_dependencies = {
	UI = {
		header = "ui.h";
		library = "ui";
	};
}

build = {
	type = "builtin",
	modules = {
		libui = {
			sources = { "libui-lua.c", "lua-compat.c", "object.c", "callback.c", "control-common.c", "controls.c", "area.c", "draw.c", "image.c", "menu.c" };
			libraries = "ui";
			incdirs = {
				"$(UI_INCDIR)";
			};
			libdirs = {
				"$(UI_LIBDIR)";
			};
		};
	};
}
