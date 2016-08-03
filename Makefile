TARGET = libui/core.so

CFLAGS = -fPIC
LDFLAGS = -shared -s

SRC = libui-lua.c

ifeq ($(OS),Windows_NT)
    TARGET = libui/core.dll
    CFLAGS += -Ic:/src/lua/stage/include -I../ -I../libui/ -DMODULE_API=__declspec\(dllexport\)
    LDFLAGS += -Lc:/src/lua/stage/lib -llua53 -L../out -lui
    SRC += libui-windows-lua.c
else
    CFLAGS += -I../libui/ -Wall -fvisibility=hidden -DMODULE_API=__attribute__\(\(visibility\(\"default\"\)\)\)
    CFLAGS += $(shell pkg-config --cflags lua5.3)
    LDFLAGS += $(shell pkg-config --libs lua5.3)
    LDFLAGS += -L../libui/out -rpath=../libui/out -lui
endif

SRC = libui-lua.c callback.c control-common.c controls.c menu.c object.c
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
