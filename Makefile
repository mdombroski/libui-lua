TARGET = libui/core.so

CFLAGS = -fPIC -g
LDFLAGS = -shared

SRC = libui-lua.c

ifeq ($(OS),Windows_NT)
    TARGET = libui/core.dll
    CFLAGS += -I../lua-5.3.3/src -I../libui/ -DMODULE_API=__declspec\(dllexport\)
    LDFLAGS += -L../lua-5.3.3/src -llua53 ../libui/out/libui.lib
    SRC += libui-windows-lua.c
else
    CFLAGS += -I../libui/ -Wall -fvisibility=hidden -DMODULE_API=__attribute__\(\(visibility\(\"default\"\)\)\)
    CFLAGS += $(shell pkg-config --cflags lua5.3)
    LDFLAGS += $(shell pkg-config --libs lua5.3)
    LDFLAGS += -L../libui/out -rpath=../libui/out -lui
endif

SRC = libui-lua.c callback.c control-common.c controls.c menu.c object.c image.c area.c draw.c
OBJ = $(SRC:%.c=%.o)


all: $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	mkdir -p libui
	$(LD) -o $(TARGET) $(OBJ) $(LDFLAGS)

clean:
	@rm $(TARGET) $(OBJ)

.PHONY : all clean
