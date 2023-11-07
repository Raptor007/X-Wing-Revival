# User can override these variables as needed:
CC = g++
LD := $(CC)
CFLAGS =
LDFLAGS =
ARCH =
MARCH = native
MCPU =
MTUNE =
MFLAGS =
O = 2
OFLAGS = -O$(O) -fomit-frame-pointer -ftree-vectorize -fno-strict-aliasing -flto
WFLAGS = -Wall -Wextra -Wno-multichar -Wno-unused-parameter
INC = /opt/local/include /opt/local/include/SDL /usr/local/include /usr/include
LIBDIR = /usr/lib64
LIB = libSDLmain.a libSDL_net.so libSDL_mixer.so libSDL_ttf.so libSDL_image.so libSDL.so libGLEW.so libGLU.so libGL.so libopenvr_api.so
DEF =
EXE = Data/xwingrev
VERSION = $(shell grep 'define VERSION' Sources/Main.cpp | grep -oh '[0-9.]*')
GAMEDIR = /Games/X-Wing Revival
SERVERDIR = /srv/xwingrev
SERVERUSER = xwingrev
MAC_CODESIGN = $(shell whoami)
MAC_FRAMEWORKS = OpenGL Cocoa AudioUnit AudioToolbox IOKit Carbon
MAC_INSTALL_NAME_TOOL = /opt/local/bin/install_name_tool
MAC_BUNDLE_LIBS = /opt/local/lib/libgcc/libstdc++.6.dylib /opt/local/lib/libgcc/libgcc_s.1.dylib /usr/local/lib/libopenvr_api.dylib
MAC_PPC_ARCH = ppc
MAC_PPC_MCPU = G3
MAC_PPC_MTUNE = G4
MAC_I32_ARCH = pentium4
MAC_I32_MARCH = nocona
MAC_I32_MTUNE = nocona
MAC_I64_ARCH = x86_64
MAC_I64_MARCH = core2
MAC_I64_MTUNE = core2


INCLUDES = Sources Sources/Game Sources/UI $(wildcard ../RaptorEngine/*) $(INC)
LIBRARIES = $(foreach lib,$(LIB),$(LIBDIR)/$(lib))
SOURCES = $(wildcard Sources/*.cpp) $(wildcard Sources/*/*.cpp) $(wildcard ../RaptorEngine/*/*.cpp)
GAME_HEADERS = $(wildcard Sources/*.h) $(wildcard Sources/*/*.h)
ENGINE_HEADERS = $(wildcard ../RaptorEngine/*/*.h)
TARGET = exe
PRODUCT = $(EXE)
XFLAGS =


UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)
# Begin Mac OS X section.

ifndef ARCH
# Unless ARCH is defined, Macs should make universal binaries by default.
TARGET = universal
ARCH = $(MAC_PPC_ARCH) $(MAC_I32_ARCH)
endif

# Don't use default march=native on Macs.
MARCH =

# Use MacPorts lib directory.
LIBDIR = /opt/local/lib

# Always suggest 10.4 Tiger as the minimum Mac OS X version.
XFLAGS += -mmacosx-version-min=10.4

ifeq (,$(findstring $(MAC_I64_ARCH),$(ARCH)))
# When building without i64 target, use the 10.4 Tiger universal SDK and ppc/i32 MacPorts libs.
XFLAGS += -isysroot /Developer/SDKs/MacOSX10.4u.sdk
LIBDIR = /Developer/SDKs/MacOSX10.4u.sdk/opt/local/lib
endif

KERNEL_VERSION = $(shell uname -r)
ifneq (8.11.0,$(KERNEL_VERSION))
ifneq ($(MAC_I64_ARCH),$(ARCH))
# When building on 10.5+ with any non-i64 target, use Apple gcc 4.0 and disable stack protector.
CC = /Developer/usr/bin/g++-4.0
LD = /Developer/usr/bin/g++-4.0
OFLAGS += -fno-stack-protector
endif
else
# When building on 10.4, specify gcc 4.0.
CC = g++-4.0
LD = g++-4.0
endif

GCC_VERSION = $(shell $(CC) -dumpversion)
GCC_VERSION_MAJOR = $(word 1,$(subst ., ,$(GCC_VERSION)))
GCC_VERSION_MINOR = $(word 2,$(subst ., ,$(GCC_VERSION)))
ifneq (,$(findstring 4.9.,$(GCC_VERSION)))
# When using GCC 4.9, don't let the linker use compact unwind.
XFLAGS += -Wl,-no_compact_unwind
else
ifneq (4,$(GCC_VERSION_MAJOR))
# When using GCC 5+, don't let the linker use compact unwind.
XFLAGS += -Wl,-no_compact_unwind
endif
endif

ifneq (,$(findstring -4.0,$(CC)))
# When using gcc 4.0, don't use link-time optimization (it's not supported) and don't use auto-vectorization because it enables strict aliasing.
OFLAGS := $(filter-out -flto,$(OFLAGS))
OFLAGS := $(filter-out -ftree-vectorize,$(OFLAGS))
# Don't warn about OpenVR's use of non-virtual destructors.
WFLAGS += -Wno-non-virtual-dtor
endif

ifeq ($(MAC_PPC_ARCH),$(ARCH))
MFLAGS =
else
# Specify SSE3/SSSE3 when building for anything other than ppc.
ifeq (,$(findstring -4.0,$(CC)))
# On gcc > 4.0, use SSSE3.
MFLAGS = -mssse3
ifneq (,$(findstring $(MAC_I64_ARCH),$(ARCH)))
# For i64 on gcc > 4.0, enable SAHF instruction.
MFLAGS += -msahf
endif
else
# On gcc 4.0, use SSE3.
MFLAGS = -msse3
endif
ifeq (,$(findstring $(MAC_PPC_ARCH),$(ARCH)))
# Specify mfpmath=sse if there's no ppc target.
MFLAGS += -mfpmath=sse
endif
endif

# Override LIB specified above for Linux.
LIB = libSDL_net.a libSDL_ttf.a libSDL_image.a libSDL_mixer.a libSDL.a libSDLmain.a libGLEW.a libmikmod.a libsmpeg.a libflac.a libpng.a libfreetype.a libvorbisfile.a libvorbis.a libogg.a libtiff.a libXrandr.a libXrender.a libXext.a libX11.a libxcb.a libXdmcp.a libXau.a libjpeg.a libbz2.a liblzma.a libz.a

# For i64 also include libbrotli (dependency of newer libfreetype).
ifneq (,$(findstring $(MAC_I64_ARCH),$(ARCH)))
LIB += libbrotlidec-static.a libbrotlicommon-static.a
endif

# Macs don't have .so files, so replace with .a files.
LIBRARIES := $(patsubst %.so,%.a,$(LIBRARIES))

# Add frameworks to Mac linker line.
LIBRARIES += $(foreach framework,$(MAC_FRAMEWORKS),-framework $(framework))

# Add openvr lib on non-PPC targets.
ifneq ($(MAC_PPC_ARCH),$(ARCH))
LIBRARIES += /usr/local/lib/libopenvr_api.dylib
endif

# Macs must pad install names so install_name_tool can make them longer.
XFLAGS += -headerpad_max_install_names

# Create application package.
TARGET = Data/X-Wing\ Revival.app
PRODUCT = Data/X-Wing Revival.app

# End Mac OS X section.
endif


# MinGW (Windows without MSVC).
ifneq (,$(findstring MINGW,$(UNAME))$(findstring CYGWIN,$(UNAME)))
LDFLAGS += -static-libgcc -static-libstdc++ -lmingw32 -lSDLmain
LIBDIR = /lib
LIB = SDL_ttf.lib SDL_image.lib SDL_mixer.lib SDL_net.lib SDL.lib openvr_api.lib glew32s.lib GLU32.lib OpenGL32.lib bufferoverflowu.lib
MFLAGS += -mwindows
LIBRARIES += -liphlpapi -ladvapi32
EXE = Data/xwingrev.exe
OBJECTS += build/XWing.res
endif


# Generate an object build directory based on the target architecture(s).
NULL :=
SPACE := $(NULL) $(NULL)
ifdef ARCH
ARCHDIR = arch-$(subst $(SPACE),-,$(ARCH))/
else
ARCHDIR = arch-native/
endif
OBJECTS += $(patsubst Sources/%.cpp,build/$(ARCHDIR)XWing/%.o,$(patsubst ../RaptorEngine/%.cpp,build/$(ARCHDIR)RaptorEngine/%.o,$(SOURCES)))
OBJDIRS += $(dir $(OBJECTS))

# Build a list of architecture flags.
AFLAGS =
ifdef ARCH
AFLAGS = $(foreach arch,$(ARCH),-arch $(arch))
endif
ifdef MARCH
AFLAGS += -march=$(MARCH)
endif
ifdef MCPU
AFLAGS += -mcpu=$(MCPU)
endif
ifdef MTUNE
AFLAGS += -mtune=$(MTUNE)
endif


# Default to SDL2 if detected.
ifndef SDL1
ifneq (,$(wildcard $(LIBDIR)/libSDL2.so))
SDL2 = y
endif
endif

ifdef SDL2
DEF += SDL2
LIBRARIES := $(subst SDL,SDL2,$(LIBRARIES))
endif


# Linux ELF binary for distribution (if SDL2 detected).
ifeq ($(UNAME),Linux)
ifdef SDL2
TARGET = Data/X-Wing\ Revival.elf
PRODUCT = Data/X-Wing Revival.elf
endif
endif


.PHONY: default exe objects clean install server-install ppc i32 i64 universal lipo

default: $(TARGET)

%.app: universal
	mkdir -p "$@/Contents/MacOS"
	cp "$(EXE)" "$@/Contents/MacOS/$(notdir $(patsubst %.app,%,$@))"
	mkdir -p "$@/Contents/Resources"
	cp -p "Info.plist" "$@/Contents/"
	rsync -ax --exclude=".*" "English.lproj" "$@/Contents/Resources/"
	cp -p "xwing128.icns" "$@/Contents/Resources/"
	$(foreach lib,$(MAC_BUNDLE_LIBS),cp -p "$(lib)" "$@/Contents/MacOS/"; chmod 644 "$@/Contents/MacOS/$(notdir $(lib))";)
	$(foreach lib,$(MAC_BUNDLE_LIBS),$(MAC_INSTALL_NAME_TOOL) -change "$(lib)" "@loader_path/$(notdir $(lib))" "$@/Contents/MacOS/$(notdir $(patsubst %.app,%,$@))";)
	$(foreach lib1,$(MAC_BUNDLE_LIBS),$(foreach lib2,$(MAC_BUNDLE_LIBS),$(MAC_INSTALL_NAME_TOOL) -change "$(lib1)" "@loader_path/$(notdir $(lib1))" "$@/Contents/MacOS/$(notdir $(lib2))";))
	-codesign -s "$(MAC_CODESIGN)" "$@"
	-if [ -L "$@/Contents/CodeResources" ]; then rm "$@/Contents/CodeResources"; rsync -ax "$@/Contents/_CodeSignature/CodeResources" "$@/Contents/"; fi

%.elf: exe
	-chmod +x "$(EXE)"
	rsync -ax "$(EXE)" "$@"
	-patchelf --replace-needed /usr/lib64/libopenvr_api.so Bin64/libopenvr_api.so "$@"
	-patchelf --replace-needed libSDL2-2.0.so.0 Bin64/libSDL2.so "$@"
	-patchelf --replace-needed libSDL2_image-2.0.so.0 Bin64/libSDL2_image.so "$@"
	-patchelf --replace-needed libSDL2_mixer-2.0.so.0 Bin64/libSDL2_mixer.so "$@"
	-patchelf --replace-needed libSDL2_net-2.0.so.0 Bin64/libSDL2_net.so "$@"
	-patchelf --replace-needed libSDL2_ttf-2.0.so.0 Bin64/libSDL2_ttf.so "$@"
	-patchelf --replace-needed libGL.so.1 Bin64/libGL.so "$@"
	-patchelf --replace-needed libGLU.so.1 Bin64/libGLU.so "$@"
	-patchelf --replace-needed libGLEW.so.2.2 Bin64/libGLEW.so "$@"
	-patchelf --replace-needed libstdc++.so.6 Bin64/libstdc++.so "$@"
	-patchelf --replace-needed libgcc_s.so.1 Bin64/libgcc_s.so "$@"
	-patchelf --replace-needed libm.so.6 Bin64/libm.so "$@"
	-patchelf --replace-needed libc.so.6 Bin64/libc.so "$@"

exe: $(SOURCES) $(GAME_HEADERS) $(ENGINE_HEADERS) $(EXE)

$(EXE): $(OBJECTS)
	$(LD) $(AFLAGS) $(MFLAGS) $(OFLAGS) $(WFLAGS) $(XFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBRARIES) -o $@

build/$(ARCHDIR)XWing/%.o: Sources/%.cpp $(GAME_HEADERS) $(ENGINE_HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -c $(AFLAGS) $(MFLAGS) $(OFLAGS) $(WFLAGS) $(XFLAGS) $(CFLAGS) $(foreach inc,$(INCLUDES),-I$(inc)) $(foreach def,$(DEF),-D$(def)) $< -o $@

build/$(ARCHDIR)RaptorEngine/%.o: ../RaptorEngine/%.cpp $(ENGINE_HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -c $(AFLAGS) $(MFLAGS) $(OFLAGS) $(WFLAGS) $(XFLAGS) $(CFLAGS) $(foreach inc,$(INCLUDES),-I$(inc)) $(foreach def,$(DEF),-D$(def)) $< -o $@

build/XWing.res: XWing.rc
	windres XWing.rc -O coff -o $@

objects: $(SOURCES) $(HEADERS) $(OBJECTS)

clean:
	rm -rf build/arch-* "$(PRODUCT)" "$(EXE)" "$(EXE)"_* build/XWing.res

install:
	mkdir -p "$(GAMEDIR)"
	-rsync -ax --exclude=".*" Data/* "$(GAMEDIR)/"
	-rsync -ax README.txt "$(GAMEDIR)/"
	rsync -ax "$(PRODUCT)" "$(GAMEDIR)/"

server-install:
	mkdir -p "$(SERVERDIR)"
	cp xwingctl "$(SERVERDIR)/"
	-"$(SERVERDIR)/xwingctl" stop
	cp "$(EXE)" "$(SERVERDIR)/$(notdir $(EXE))-$(VERSION)"
	cd "$(SERVERDIR)" && ln -sf "$(notdir $(EXE))-$(VERSION)" "$(notdir $(EXE))"
	-rsync -ax --exclude=".*" Data/Ships "$(SERVERDIR)/"
	-rsync -ax --exclude=".*" Data/Models "$(SERVERDIR)/"
	-chown -R $(SERVERUSER):wheel "$(SERVERDIR)"
	-chmod 775 "$(SERVERDIR)/$(notdir $(EXE))-$(VERSION)"
	-ln -sf "$(SERVERDIR)/xwingctl" /usr/local/bin/xwingctl
	-"$(SERVERDIR)/xwingctl" start

ppc:
	make objects ARCH="$(MAC_PPC_ARCH)" MCPU="$(MAC_PPC_MCPU)" MTUNE="$(MAC_PPC_MTUNE)"
	make exe EXE="$(EXE)_ppc" ARCH="$(MAC_PPC_ARCH)" MARCH="$(MAC_PPC_MARCH)" MTUNE="$(MAC_PPC_MTUNE)"

i32:
	make objects ARCH="$(MAC_I32_ARCH)" MARCH="$(MAC_I32_MARCH)" MTUNE="$(MAC_I32_MTUNE)"
	make exe EXE="$(EXE)_i32" ARCH="$(MAC_I32_ARCH)" MARCH="$(MAC_I32_MARCH)" MTUNE="$(MAC_I32_MTUNE)"

i64:
	make objects ARCH="$(MAC_I64_ARCH)" MARCH="$(MAC_I64_MARCH)" MTUNE="$(MAC_I64_MTUNE)" CC="/opt/local/bin/g++"
	make exe EXE="$(EXE)_i64" ARCH="$(MAC_I64_ARCH)" MARCH="$(MAC_I64_MARCH)" MTUNE="$(MAC_I64_MTUNE)" CC="/opt/local/bin/g++"

universal:
	make ppc EXE="$(EXE)"
	make i32 EXE="$(EXE)"
	make i64 EXE="$(EXE)"
	make lipo EXE="$(EXE)"

lipo: $(EXE)_ppc $(EXE)_i32 $(EXE)_i64
	lipo $(EXE)_ppc $(EXE)_i32 $(EXE)_i64 -create -output $(EXE)
