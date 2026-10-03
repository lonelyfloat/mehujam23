CC = gcc
CXX = g++
CMAKE = cmake

PROGRAM_NAME=game
EXE_NAME=game
PLATFORM = PLATFORM_DESKTOP
BUILD_MODE=DEBUG
DEFAULT_TARGET=release
DEPENDENCY_TARGET=static

SOURCE_FILES = $(shell find src -name *.c -a ! -name main.c)
MAIN_FILES = src/main.c 

CFLAGS= -Wall -Ivendor/SDL3/include -Ivendor/SDL_mixer/include
LDFLAGS= 
LDLIBS= -lGL -lm -lpthread -ldl -lrt -lX11 -lSDL3 -lSDL3_mixer

INCLUDE_IMGUI=ON
IMGUI_DIR = vendor/imgui
DCIMGUI_DIR = vendor/dear_bindings
IMGUI_FILES += $(shell find $(IMGUI_DIR) -maxdepth 1 -name *.cpp)
IMGUI_FILES += $(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp
IMGUI_FILES += $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer3.cpp
IMGUI_FILES += $(shell find $(DCIMGUI_DIR) -maxdepth 1 -name *.cpp)

ifeq ($(PLATFORM), PLATFORM_WEB)
	CC = emcc
	CXX = em++
	MAKE= emmake make
	CMAKE = emcmake cmake
	EXE_NAME = $(PROGRAM_NAME)_web/index.html
	BUILD_MODE=RELEASE
	DEPENDENCY_TARGET=static
	CFLAGS+= -s USE_GLFW=3 -DPLATFORM_WEB --preload-file assets --shell-file vendor/minshell.html
endif

ifeq ($(BUILD_MODE), DEBUG)
	CFLAGS += -DDEBUG_MODE
	DEFAULT_TARGET=hot-reload-lib
	DEPENDENCY_TARGET=shared
else
	DEFAULT_TARGET=release
	DEPENDENCY_TARGET=static
endif

LDFLAGS += -Lbuild/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL3
LDFLAGS += -Lbuild/$(PLATFORM)
LDFLAGS += -Lbuild/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL_mixer

ifeq ($(INCLUDE_IMGUI), ON)
	# MAIN_FILES += $(IMGUI_FILES) 
	LDLIBS += -lstdc++
	CFLAGS += -I$(IMGUI_DIR)
	CFLAGS += -I$(IMGUI_DIR)/backends
	CFLAGS += -I$(DCIMGUI_DIR)
else 

endif


.PHONY: all release hot-reload-lib hot-reload-program clean dependencies

all:
	$(MAKE) $(DEFAULT_TARGET)

release: 
	$(CC) $(SOURCE_FILES) $(MAIN_FILES) $(IMGUI_FILES) -o build/$(EXE_NAME) $(CFLAGS) $(LDFLAGS) $(LDLIBS)

hot-reload-lib: 
	$(CC) -shared -fPIC $(SOURCE_FILES) -o build/$(PROGRAM_NAME).so $(CFLAGS) -Wl,-rpath=build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL3$(LDFLAGS) $(LDLIBS)

hot-reload-program:
	$(MAKE) hot-reload-lib
	$(CC) $(MAIN_FILES) src/framework/game_api.c src/framework/resource.c src/framework/audio.c src/framework/arena.c src/framework/dev_imgui.c -o build/$(PROGRAM_NAME)_debug -Wl,-rpath=build/$(PLATFORM),-rpath=build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL_mixer $(CFLAGS) -DHOT_RELOAD_LIB=build/$(PROGRAM_NAME).so $(LDFLAGS) $(LDLIBS)  -limgui

clean:
	rm -rf build
	mkdir build
	mkdir build/PLATFORM_WEB
	mkdir build/PLATFORM_WEB/static
	mkdir build/PLATFORM_DESKTOP
	mkdir build/PLATFORM_DESKTOP/static
	mkdir build/PLATFORM_DESKTOP/shared
	mkdir build/$(PROGRAM_NAME)_web

static-dependencies: 
	$(CMAKE) -S vendor/SDL3 -B build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL3 -DSDL_STATIC=ON -DSDL_TEST_LIBRARY=OFF -DSDL_EXAMPLES=OFF # build SDL
	$(MAKE) -C build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL3 -j4 # build the rest of SDL
	$(CMAKE) -S vendor/SDL_mixer -B build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL_mixer  -DSDL3_DIR=$(PWD)/build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL3 -DSDLMIXER_EXAMPLES=OFF -DBUILD_SHARED_LIBS=OFF -DSDLMIXER_VENDORED=ON -DSDLMIXER_GME=OFF -DSDLMIXER_WAVPACK=OFF   -DSDLMIXER_MOD=OFF -DSDLMIXER_MOD_XMP=OFF -DSDLMIXER_OPUS=OFF -DSDLMIXER_MP3_MPG123=OFF -DSDLMIXER_VORBIS_VORBISFILE=OFF -DSDLMIXER_FLAC_LIBFLAC=OFF
	$(MAKE) -C build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL_mixer -j4 # SDL_mixer static build

shared-dependencies:
	$(CMAKE) -S vendor/SDL3 -B build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL3 -DSDL_SHARED=ON -DSDL_TEST_LIBRARY=OFF -DSDL_EXAMPLES=OFF # build SDL shared
	$(MAKE) -C build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL3 -j4 # build the rest of SDL shared
	$(CMAKE) -S vendor/SDL_mixer -B build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL_mixer -DSDLMIXER_VENDORED=ON -DSDLMIXER_GME=OFF -DSDLMIXER_WAVPACK=OFF  -DSDLMIXER_EXAMPLES=OFF -DSDLMIXER_MOD=OFF -DSDLMIXER_MOD_XMP=OFF -DSDLMIXER_OPUS=OFF -DSDLMIXER_MP3_MPG123=OFF -DSDLMIXER_VORBIS_VORBISFILE=OFF -DSDLMIXER_FLAC_LIBFLAC=OFF
	$(MAKE) -C build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL_mixer -j4 # SDL_mixer shared build

dependencies:
	rm -rf build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL3
	rm -rf build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL_mixer
	mkdir build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL3
	mkdir build/$(PLATFORM)/$(DEPENDENCY_TARGET)/SDL_mixer
	$(MAKE) $(DEPENDENCY_TARGET)-dependencies
	$(CC) -shared $(IMGUI_FILES) -o build/$(PLATFORM)/libimgui.so $(CFLAGS) $(LDFLAGS) -lGL -lm -lpthread -ldl -lrt -lX11 -fPIC  # make runtime version of imgui
