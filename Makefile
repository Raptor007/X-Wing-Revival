# User can override these variables as needed:
CC = g++
LD := $(CC)
CFLAGS = -O3 -fstrict-aliasing -ftree-vectorize -msse3 -fomit-frame-pointer -Wall -Wno-multichar
LDFLAGS = -O3 -fstrict-aliasing -ftree-vectorize -msse3 -fomit-frame-pointer -Wall
ARCH =
MTUNE = native
INC = /opt/local/include /opt/local/include/SDL
LIBDIR = /usr/lib64
LIB = libSDLmain.a libSDL_net.so libSDL_mixer.so libSDL_ttf.so libSDL_image.so libSDL.so libGLEW.a libGLU.so libGL.so
DEF =
EXE = xwingrev
VERSION = 0.1.1
GAMEDIR = /Games/X-Wing Revival
SERVERDIR = /srv/xwingrev
SERVERUSER = xwingrev
BUILD_FINALIZE =
INSTALL_FINALIZE =
MAC_APP = X-Wing Revival
MAC_FRAMEWORKS = OpenGL Cocoa AudioUnit AudioToolbox IOKit Carbon
MAC_MFLAGS = -fobjc-direct-dispatch
MAC_INSTALL_NAME_TOOL = /opt/local/bin/install_name_tool


CARCH =
LDARCH =
INCLUDES = Sources Sources/Game Sources/UI $(wildcard ../RaptorEngine/*) $(INC)
LIBRARIES = $(foreach lib,$(LIB),$(LIBDIR)/$(lib))
SOURCES = $(wildcard Sources/*.cpp) $(wildcard Sources/*/*.cpp) $(wildcard ../RaptorEngine/*/*.cpp)
HEADERS = $(wildcard Sources/*.h) $(wildcard Sources/*/*.h) $(wildcard ../RaptorEngine/*/*.h)
EXE_INSTALL = $(EXE)
BINDIR = $(GAMEDIR)


UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)
# Begin Mac OS X section.

ifndef ARCH
# Unless ARCH is defined, Macs should make 32-bit universal binaries by default.
ARCH = ppc i386
endif

# Unless overridden by user, don't use mtune=native because gcc 4.0 doesn't understand it.
MTUNE =

# Use MacPorts lib directory.
LIBDIR = /opt/local/lib

# Macs must pad install names so install_name_tool can make them longer.
LDFLAGS += -headerpad_max_install_names

# Always suggest 10.4 Tiger as the minimum Mac OS X version.
CFLAGS += -mmacosx-version-min=10.4
LDFLAGS += -mmacosx-version-min=10.4

ifeq (,$(findstring x86_64,$(ARCH)))
# When building without x86_64 target, use the 10.4 Tiger universal SDK and ppc/i386 MacPorts libs.
CFLAGS += -isysroot /Developer/SDKs/MacOSX10.4u.sdk
LDFLAGS += -isysroot /Developer/SDKs/MacOSX10.4u.sdk
LIBDIR = /Developer/SDKs/MacOSX10.4u.sdk/opt/local/lib
endif

KERNEL_VERSION = $(shell uname -r)
ifneq (8.11.0,$(KERNEL_VERSION))
# When building on 10.5+, disable stack protector because 10.4 doesn't have those symbols.
CFLAGS += -fno-stack-protector
LDFLAGS += -fno-stack-protector
ifneq (x86_64,$(ARCH))
# When building on 10.5+ with any non-x86_64 target, use Apple gcc 4.0.
CC = /Developer/usr/bin/g++-4.0
LD = /Developer/usr/bin/g++-4.0
endif
else
# When building on 10.4, specify gcc 4.0.
CC = g++-4.0
LD = g++-4.0
endif

# Override LIB specified above for Linux.
LIB =

ifndef FRAMEWORKS_INSTEAD_OF_LIBS
# By default, statically-link as many libraries as possible from MacPorts.
LIB += libSDL_net.a libSDL_ttf.a libSDL_image.a libSDL_mixer.a libSDL.a libSDLmain.a
else
# When building for 10.4 Tiger on a newer system without compatible libs, we might need to use SDLMain.m and Frameworks instead.
SOURCES += ../RaptorEngine/Core/SDLMain.m
MAC_FRAMEWORKS += GLEW SDL_net SDL_ttf SDL_image SDL_mixer SDL mikmod smpeg
endif

# These libraries should always be statically-linked.
LIB += libGLEW.a libmikmod.a libsmpeg.a libflac.a libpng.a libfreetype.a libvorbisfile.a libvorbis.a libogg.a libtiff.a libXrandr.a libXrender.a libXext.a libX11.a libxcb.a libXdmcp.a libXau.a libjpeg.a libbz2.a liblzma.a libz.a

# Macs don't have .so files, so replace with .a files.
LIBRARIES := $(patsubst %.so,%.a,$(LIBRARIES))

# Add frameworks to Mac linker line.
LIBRARIES += $(foreach framework,$(MAC_FRAMEWORKS),-framework $(framework))

# Fix dynamic library paths.
BUILD_FINALIZE += $(MAC_INSTALL_NAME_TOOL) -change "@loader_path/Frameworks/mikmod.framework/Versions/A/mikmod" "@executable_path/../Frameworks/mikmod.framework/Versions/A/mikmod" "$(EXE)";
BUILD_FINALIZE += $(MAC_INSTALL_NAME_TOOL) -change "@loader_path/Frameworks/smpeg.framework/Versions/A/smpeg" "@executable_path/../Frameworks/smpeg.framework/Versions/A/smpeg" "$(EXE)";

ifdef MAC_APP
# Create application package.
EXE_INSTALL = $(MAC_APP)
BINDIR = $(GAMEDIR)/$(MAC_APP).app/Contents/MacOS
INSTALL_FINALIZE += mkdir -p "$(GAMEDIR)/$(MAC_APP).app/Contents/Resources"; cp -p "Info.plist" "$(GAMEDIR)/$(MAC_APP).app/Contents/"; rsync -ax --exclude=".*" "English.lproj" "$(GAMEDIR)/$(MAC_APP).app/Contents/Resources/"; cp -p "xwing128.icns" "$(GAMEDIR)/$(MAC_APP).app/Contents/Resources/";
endif

# End Mac OS X section.
endif


# Generate an object build directory based on the target architecture(s).
NULL :=
SPACE := $(NULL) $(NULL)
ifdef ARCH
ARCHDIR = arch-$(subst $(SPACE),-,$(ARCH))/
else
ARCHDIR = arch-native/
endif
OBJECTS = $(patsubst Sources/%.cpp,build/$(ARCHDIR)XWing/%.o,$(patsubst ../RaptorEngine/%.cpp,build/$(ARCHDIR)RaptorEngine/%.o,$(patsubst ../RaptorEngine/%.m,build/$(ARCHDIR)RaptorEngine/%.om,$(SOURCES))))
OBJDIRS = $(dir $(OBJECTS))

ifeq (,$(findstring -4.0,$(CC)))
# When not using gcc 4.0, use link-time optimization.
CFLAGS += -flto
LDFLAGS += -flto
endif

ifdef ARCH
# Add compiler/linker flags for arch as CARCH/LDARCH so they don't get overridden by CFLAGS/LDFLAGS.
CARCH = $(foreach arch,$(ARCH),-arch $(arch))
LDARCH = $(foreach arch,$(ARCH),-arch $(arch))
endif

ifdef MTUNE
# Add mtune to CARCH/LDARCH so they don't get overridden by CFLAGS/LDFLAGS.
CARCH += -mtune=$(MTUNE)
LDARCH += -mtune=$(MTUNE)
endif


default: $(SOURCES) $(HEADERS) $(EXE)

$(EXE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(LDARCH) $(OBJECTS) $(LIBRARIES) -o $@
	-$(BUILD_FINALIZE)

build/$(ARCHDIR)XWing/%.o: Sources/%.cpp $(HEADERS) $(foreach objdir,$(OBJDIRS),$(objdir).dir)
	$(CC) -c $(CARCH) $(CFLAGS) $(foreach inc,$(INCLUDES),-I$(inc)) $(foreach def,$(DEF),-D$(def)) $< -o $@

build/$(ARCHDIR)RaptorEngine/%.o: ../RaptorEngine/%.cpp $(HEADERS) $(foreach objdir,$(OBJDIRS),$(objdir).dir)
	$(CC) -c $(CARCH) $(CFLAGS) $(foreach inc,$(INCLUDES),-I$(inc)) $(foreach def,$(DEF),-D$(def)) $< -o $@

build/$(ARCHDIR)RaptorEngine/%.om: ../RaptorEngine/%.m $(HEADERS) $(foreach objdir,$(OBJDIRS),$(objdir).dir)
	$(CC) -c $(CARCH) $(CFLAGS) $(MAC_MFLAGS) $(foreach inc,$(INCLUDES),-I$(inc)) $< -o $@

build/%/.dir:
	mkdir -p $(dir $@)
	touch $@

objects: $(SOURCES) $(HEADERS) $(OBJECTS)

clean:
	rm -rf build/arch-* "$(EXE)" "$(EXE)"_*

install:
	mkdir -p "$(BINDIR)"
	cp "$(EXE)" "$(BINDIR)/$(EXE_INSTALL)"
	cp -p build/Debug/README.txt "$(GAMEDIR)/"
	cp -p build/Debug/icon.bmp "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Fonts "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Textures "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Models "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Shaders "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Sounds "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Music "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Screensaver "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Tools "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Docs "$(GAMEDIR)/"
	chmod ugo+x "$(BINDIR)/$(EXE_INSTALL)"
	$(INSTALL_FINALIZE)

uninstall:
	rm -r "$(GAMEDIR)"

play:
	cd "$(GAMEDIR)"; "$(BINDIR)/$(EXE_INSTALL)"

server-install:
	mkdir -p "$(SERVERDIR)"
	cp "$(EXE)" "$(SERVERDIR)/$(EXE)-$(VERSION)"
	cp xwingctl "$(SERVERDIR)/"
	cd "$(SERVERDIR)" && ln -sf "$(EXE)-$(VERSION)" "$(EXE)"
	-chown -R $(SERVERUSER):wheel "$(SERVERDIR)"
	-chmod 775 "$(SERVERDIR)/$(EXE)-$(VERSION)"
	-ln -sf "$(SERVERDIR)/xwingctl" /usr/local/bin/xwingctl

ppc:
	make objects ARCH="ppc" MTUNE="G5"
	make ARCH="ppc" EXE="$(EXE)_ppc" MTUNE="G5"

i32:
	make objects ARCH="i386" MTUNE="nocona"
	make ARCH="i386" EXE="$(EXE)_i32" MTUNE="nocona"

i64:
	make objects ARCH="x86_64" CC="/opt/local/bin/g++" MTUNE="core2"
	make ARCH="x86_64" EXE="$(EXE)_i64" CC="/opt/local/bin/g++" MTUNE="core2"

universal:
	make ppc EXE="$(EXE)"
	make i32 EXE="$(EXE)"
	make i64 EXE="$(EXE)"
	make lipo EXE="$(EXE)"

lipo: $(EXE)_ppc $(EXE)_i32 $(EXE)_i64
	lipo $(EXE)_ppc $(EXE)_i32 $(EXE)_i64 -create -output $(EXE)
