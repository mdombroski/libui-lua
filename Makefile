TARGET = libui/core.dll

CFLAGS = -Ic:/src/lua/stage/include -I../
LDFLAGS = -shared -s -Lc:/src/lua/stage/lib -llua53 -L../out -lui

ifeq ($(OS),Windows_NT)
    CFLAGS += -DMODULE_API=__declspec\(dllexport\)
else
    CFLAGS += -fvisibility=hidden -DMODULE_API=__attribute__\(\(visibility\(\"default\"\)\)\)
endif

SRC = libui-lua.c libui-windows-lua.c
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
