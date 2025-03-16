# prong build script (JAN 17 2025)
# VERSION 2.9 (MAR 16 2025)

# Compiler and linker
CC = gcc

EXTRADEFS = -Isrc/quake -Isrc/quake-win -Isrc/resource/ 

BUILD ?= DEBUG

# 64 bit

# can be x64 or i386. i386 is quake's original target
ARCH = x64

# quake.. you know.. uses an older version of c.. but since im such a nice guy im using new c
# with warning suppression to make gcc be quiet
IGNOREWARNINGS = -Wpointer-to-int-cast -Wno-implicit -Wimplicit-function-declaration

CFLAGS = $(IGNOREWARNINGS) -fcommon $(EXTRADEFS) -Dstricmp=strcasecmp -mno-tls-direct-seg-refs -DGLQUAKE


LDFLAGS = -lSDL2 -lGL -fcommon -mno-tls-direct-seg-refs -lm -lGLU -lGLEW

CFLAGS += -DUSE_SDL2
# CFLAGS += -DDO_USERDIRS=1
# hack
CFLAGS += -DSDL_FRAMEWORK 

# tell the compiler we mean it!!
ifeq ($(ARCH),i386)
CFLAGS += -m32
LDFLAGS += -m32
endif


mp3_obj=snd_mp3
lib_mp3dec=-lmad
cpp_vorbisdec=
lib_vorbisdec=-lvorbisfile -lvorbis -logg
CFLAGS+= -DUSE_CODEC_VORBIS $(cpp_vorbisdec)
LDFLAGS+= $(lib_vorbisdec)
CFLAGS+= -DUSE_CODEC_MP3
LDFLAGS+= $(lib_mp3dec)


# linux doesnt need an outextension
OUTEXTENSION = 


QW_SERVER_APPNAME = qwds
QW_GLCLIENT_APPNAME = quakesauce

PRONG_APPNAME = $(QW_GLCLIENT_APPNAME)

# release flags.. go for -O 1-3 or s... or nothing..
GCFLAGS = -O2
SRC_FOLDER = src/quake
# Source files
SRC = $(wildcard $(SRC_FOLDER)/*.c)

CFLAGS += -MMD -MP

# Directories
BUILD_DIR = build
RELEASE_DIR = $(BUILD_DIR)/release
RELEASES_DIR = $(BUILD_DIR)/release-s
EDIT_DIR = $(BUILD_DIR)/edit
DEBUG_DIR = $(BUILD_DIR)/debug

RELEASE_SUFFIX = -r
DEBUG_SUFFIX = -d
EDIT_SUFFIX = -t
RELEASES_SUFFIX = -rs

DEBUG_OUTNAME = $(BUILD_DIR)/$(PRONG_APPNAME)$(DEBUG_SUFFIX)$(OUTEXTENSION)
RELEASE_OUTNAME = $(BUILD_DIR)/$(PRONG_APPNAME)$(RELEASE_SUFFIX)$(OUTEXTENSION)
EDIT_OUTNAME = $(BUILD_DIR)/$(PRONG_APPNAME)$(EDIT_SUFFIX)$(OUTEXTENSION)

ifeq ($(BUILD),DEBUG)
# debug cflags
CFLAGS += -DDEBUG=1 -gdwarf-4 -g3
CUR_BUILD_DIR = $(DEBUG_DIR)
CUR_OUTNAME = $(DEBUG_OUTNAME)

else ifeq ($(BUILD),RELEASE)
# release cflags
CFLAGS += $(GCFLAGS)
CUR_BUILD_DIR = $(RELEASE_DIR)
CUR_OUTNAME = $(RELEASE_OUTNAME)

else ifeq ($(BUILD),EDIT)
# edit cflags
CFLAGS += -DDEBUG=1 -DPRONG_TOOL=EDIT -gdwarf-4 -g3
CUR_BUILD_DIR = $(EDIT_DIR)
CUR_OUTNAME = $(EDIT_OUTNAME)

endif

# Ensure the build directory exists
$(CUR_BUILD_DIR):
	mkdir -p $(CUR_BUILD_DIR)

# Corrected object and dependency file paths
OBJECTS = $(patsubst $(SRC_FOLDER)/%.c, $(CUR_BUILD_DIR)/%.o, $(SRC))
DEPS = $(OBJECTS:.o=.d)

# Include dependencies properly
-include $(DEPS)

RESOURCE_OBJECTS = $(CUR_BUILD_DIR)/prongresource.o $(CUR_BUILD_DIR)/resourcelayout.o 

# Build rules
$(CUR_OUTNAME): $(RESOURCE_OBJECTS) $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(RESOURCE_OBJECTS) -o $(CUR_OUTNAME) $(LDFLAGS)

$(CUR_BUILD_DIR)/%.o: $(SRC_FOLDER)/%.c | $(CUR_BUILD_DIR)
	@echo "\e[1m ** Compiling $@ from $< \e[0m"
	$(CC) $(CFLAGS) -c $< -o $@

$(CUR_BUILD_DIR)/%.o: src/resource/%.S | $(CUR_BUILD_DIR)
	@echo "\e[1m ** Assembling $@ from $< \e[0m"
	$(CC) $(CFLAGS) -c $< -o $@

$(CUR_BUILD_DIR)/%.o: src/resource/%.c | $(CUR_BUILD_DIR)
	@echo "\e[1m ** Compiling $@ from $< \e[0m"
	$(CC) $(CFLAGS) -c $< -o $@

all: $(CUR_OUTNAME)

# Clean rule
clean:
	rm -f $(OBJECTS) $(DEPS) $(CUR_OUTNAME)

.PHONY: all release edit debug asm clean_release clean_edit clean_debug clean_asm clean everything build_resource clean_resource symbols build_releasewsymbols allsymbols
