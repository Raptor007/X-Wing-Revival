# User can override these variables as needed:
CC = g++
CFLAGS = -O3 -fopenmp -Wall -Wno-multichar
LDFLAGS = -fopenmp
INC = /opt/local/include /opt/local/include/SDL
LIBDIR = /usr/lib64
LIB = libSDLmain.a libSDL_net.so libSDL_mixer.so libSDL_ttf.so libSDL_image.so libSDL.so libGLEW.a libGLU.so libGL.so
EXE = xwingrev
GAMEDIR = /Games/X-Wing Revival
INSTALL_FINALIZE =
MAC_LIB = /Library/Frameworks/GLEW.framework/GLEW /System/Library/Frameworks/OpenGL.framework/OpenGL /Library/Frameworks/SDL_net.framework/SDL_net /Library/Frameworks/SDL_ttf.framework/SDL_ttf /Library/Frameworks/SDL_image.framework/SDL_image /Library/Frameworks/SDL_mixer.framework/SDL_mixer /Library/Frameworks/SDL_mixer.framework/Frameworks/mikmod.framework/mikmod /Library/Frameworks/SDL_mixer.framework/Frameworks/smpeg.framework/smpeg /Library/Frameworks/SDL.framework/SDL /System/Library/Frameworks/Cocoa.framework/Cocoa /System/Library/Frameworks/AudioUnit.framework/AudioUnit /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox /System/Library/Frameworks/IOKit.framework/IOKit /System/Library/Frameworks/Carbon.framework/Carbon
MAC_SRC = ../RaptorEngine/Core/SDLMain.m
MAC_APP = X-Wing Revival
ARCH = ppc i386 x86_64


CARCH =
LDARCH =
INCLUDES = Sources Sources/Game Sources/UI $(wildcard ../RaptorEngine/*) $(INC)
LIBRARIES = $(foreach lib,$(LIB),$(LIBDIR)/$(lib))
SOURCES = $(wildcard Sources/*.cpp) $(wildcard Sources/*/*.cpp) $(wildcard ../RaptorEngine/*/*.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(patsubst %.m,%.o,$(SOURCES)))
EXE_INSTALL = $(EXE)
BINDIR = $(GAMEDIR)

UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)
# Begin Mac OS X section.

CPU = $(shell uname -p)

ifndef ARCH
ifeq ($(CPU), i386)
# Undefined ARCH on i386/x86_64 should just build i386.
ARCH = i386
endif
endif

ifdef ARCH
CARCH = $(foreach arch,$(ARCH),-arch $(arch))
LDARCH = $(foreach arch,$(ARCH),-arch $(arch))
ifeq ($(ARCH), ppc)
ifeq ($(CPU), powerpc)
# If ARCH=ppc and CPU=powerpc, it's native; assume we don't need the frameworks.
NATIVE = true
endif
endif
ifeq ($(ARCH), x86_64)
ifeq ($(CPU), i386)
# If ARCH=x86_64 and CPU=i386, it's probably native; assume we don't need the frameworks.
NATIVE = true
endif
endif
else
# No ARCH defined, so it's native; assume we don't need the frameworks.
NATIVE = true
endif

# Use MacPorts directories.
LIBDIR = /opt/local/lib
LIB = libGLEW.a libSDL_net.a libSDL_ttf.a libSDL_image.a libSDL_mixer.a libSDLmain.a libSDL.a libmikmod.a libsmpeg.a libflac.a libpng.a libjpeg.a libtiff.a libfreetype.a libxcb.a libXdmcp.a libXrandr.a libXrender.a libXau.a libXext.a libX11.a libbz2.a libz.a

# Macs don't have .so files, so replace with .a files.
LIBRARIES := $(patsubst %.so,%.a,$(LIBRARIES))

# Include Mac-specific libraries (Frameworks) and sources (SDLMain.m).
LIBRARIES += $(MAC_LIB)
SOURCES += $(MAC_SRC)

ifdef MAC_APP
EXE_INSTALL = $(MAC_APP)
BINDIR = $(GAMEDIR)/$(MAC_APP).app/Contents/MacOS
#INSTALL_FINALIZE += mkdir -p "$(GAMEDIR)/$(MAC_APP).app/Contents/Resources"; cp -p "Info.plist" "$(GAMEDIR)/$(MAC_APP).app/Contents/"; rsync -ax --exclude=".*" "English.lproj" "/$(GAMEDIR)/$(MAC_APP).app/Contents/Resources";
INSTALL_FINALIZE += mkdir -p "$(GAMEDIR)/$(MAC_APP).app/Contents/Frameworks"; rsync -ax /Library/Frameworks/SDL_mixer.framework/Frameworks/smpeg.framework "$(GAMEDIR)/$(MAC_APP).app/Contents/Frameworks"; rsync -ax /Library/Frameworks/SDL_mixer.framework/Frameworks/mikmod.framework "$(GAMEDIR)/$(MAC_APP).app/Contents/Frameworks";
INSTALL_FINALIZE += cd "$(GAMEDIR)"; ln -shf "$(MAC_APP).app/Contents" "$(GAMEDIR)/@loader_path";
else
INSTALL_FINALIZE += cd "$(GAMEDIR)"; ln -shf "/Library/Frameworks/SDL_mixer.framework" "$(GAMEDIR)/@loader_path";
endif

# End Mac OS X section.
endif


default: $(SOURCES) $(EXE)

$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LDARCH) $(OBJECTS) $(LIBRARIES) -o $@

.cpp.o .m.o:
	$(CC) -c $(CFLAGS) $(CARCH) $(foreach inc,$(INCLUDES),-I$(inc)) $< -o $@

clean:
	rm -rf Sources/*.o Sources/*/*.o ../RaptorEngine/*/*.o "$(EXE)"

install:
	mkdir -p "$(BINDIR)"
	cp "$(EXE)" "$(BINDIR)/$(EXE_INSTALL)"
	cp -p build/Debug/README.txt "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Fonts "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Textures "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Models "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Shaders "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Sounds "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Music "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Screensaver "$(GAMEDIR)/"
	rsync -ax --exclude=".*" build/Debug/Tools "$(GAMEDIR)/"
	chmod ugo+x "$(BINDIR)/$(EXE_INSTALL)"
	$(INSTALL_FINALIZE)

play: $(EXE) install
	cd "$(GAMEDIR)"; "$(BINDIR)/$(EXE_INSTALL)"

server-install:
	mkdir -p "$(GAMEDIR)"
	cp "$(EXE)" "$(GAMEDIR)/"


# Old method of building with multiple/specific arch, no longer needed:

$(EXE)_ppc: ppc
ppc:
	make clean
	make ARCH=ppc
	mv "$(EXE)" "$(EXE)_ppc"

$(EXE)_i386: i386
i386:
	make clean
	make ARCH=i386
	mv "$(EXE)" "$(EXE)_i386"

$(EXE)_x86_64: x86_64
x86_64:
	make clean
	make ARCH=x86_64
	mv "$(EXE)" "$(EXE)_x86_64"

$(EXE)_ppc64: ppc64
ppc64:
	make clean
	make ARCH=ppc64
	mv "$(EXE)" "$(EXE)_ppc64"

universal: $(EXE)_ppc $(EXE)_i386
	lipo -create -arch ppc "$(EXE)_ppc" -arch i386 "$(EXE)_i386" -output $(EXE)

universal-64: $(EXE)_ppc $(EXE)_i386 $(EXE)_x86_64
	lipo -create -arch ppc "$(EXE)_ppc" -arch i386 "$(EXE)_i386" -arch x86_64 "$(EXE)_x86_64" -output $(EXE)

just-lipo:
	lipo -create -arch ppc "$(EXE)_ppc" -arch i386 "$(EXE)_i386" -output $(EXE)

just-lipo-64:
	lipo -create -arch ppc "$(EXE)_ppc" -arch i386 "$(EXE)_i386" -arch x86_64 "$(EXE)_x86_64" -output $(EXE)
