#!/bin/sh

DEPENDENCIES="zlib bzip2 xz xorg-libXau xorg-libXdmcp xcb jpeg xorg-libX11 xorg-libXext Xrender Xrandr freetype tiff libpng flac libogg libvorbis smpeg libmikmod libsdl libsdl_mixer libsdl_image libsdl_ttf libsdl_net glew"

for DEPENDENCY in $DEPENDENCIES
do
	echo "Installing $DEPENDENCY..."
	sudo port -p install $DEPENDENCY +universal
done
