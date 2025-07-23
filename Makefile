TARGET_OS ?= Windows

ifeq ($(TARGET_OS), Windows)
CC = x86_64-w64-mingw32-gcc
else
CC = clang
endif
CFLAGS = -std=c11 -O3 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS += -Wno-pointer-arith -Wno-unused-parameter
CFLAGS += -Ilib/cglm/include -Ilib/glad/include -Ilib/glfw/include -Ilib/stb -Ilib/noise
LDFLAGS = lib/glad/src/glad.o lib/cglm/libcglm.a lib/glfw/src/libglfw3.a

# GLFW required frameworks on OSX
ifeq ($(UNAME_S), Darwin)
	LDFLAGS += -framework OpenGL -framework IOKit -framework CoreVideo -framework Cocoa
endif

ifeq ($(TARGET_OS), Linux)
	LDFLAGS += -ldl -lpthread
endif

ifeq ($(TARGET_OS), Windows)
	LDFLAGS += -static -lmingw32 -mconsole -lopengl32 -lgdi32 -luser32 -lkernel32 -lwinmm -lshell32 -lws2_32 lib/noise/libnoise.a -lm -lpthread
endif

RES_FILE = game.res

SRC = $(wildcard src/*.c) $(wildcard src/block/*.c) $(wildcard src/entity/*.c) $(wildcard src/gfx/*.c) $(wildcard src/ui/*.c) $(wildcard src/util/*.c) $(wildcard src/world/*.c) $(wildcard src/world/gen/*.c) lib/noise/noise1234.c src/world/threadpool.c
OBJ  = $(SRC:.c=.o)
BIN = bin

.PHONY: all clean

all: dirs libs game

libs:
ifeq ($(TARGET_OS), Windows)
	cd lib/cglm && cmake . -DCGLM_STATIC=ON && make
	cd lib/glad && $(CC) -o src/glad.o -Iinclude -c src/glad.c
	cd lib/glfw && cmake . -DGLFW_USE_OSMESA=OFF -DGLFW_USE_WAYLAND=OFF -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc && make
else
	cd lib/cglm && cmake . -DCGLM_STATIC=ON && make
	cd lib/glad && $(CC) -o src/glad.o -Iinclude -c src/glad.c
	cd lib/glfw && cmake . && make
endif

dirs:
	mkdir -p ./$(BIN)
	cp -r res ./$(BIN)

run: all
	$(BIN)/game

game: $(OBJ)
ifeq ($(TARGET_OS), Windows)
	x86_64-w64-mingw32-windres game.rc -O coff -o $(RES_FILE)
	$(CC) -o $(BIN)/game.exe $^ $(RES_FILE) $(LDFLAGS)
else
	$(CC) -o $(BIN)/game $^ $(LDFLAGS)
endif

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

lib/noise/noise1234.o: lib/noise/noise1234.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(BIN) $(OBJ) $(RES_FILE)
