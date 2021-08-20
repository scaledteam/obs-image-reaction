# OBS Image Reaction Plugin
Image that reacts to sound source. It change picture from one to another based on volume.

## Installing binaries
Download binaries from [Releases](https://github.com/scaledteam/obs-image-reaction/releases/). 

For Windows, put "libimage-reaction.dll" into "C:\Program Files\obs-studio\obs-plugins\64bit\" folder.

For GNU/Linux, put "libimage-reaction" folder into "~/.config/obs-studio/plugins" folder.

## Building and installing for GNU/Linux:
```
git clone https://github.com/scaledteam/obs-image-reaction
cd obs-image-reaction
mkdir build
cd build
cmake ..
make
mkdir -p ~/.config/obs-studio/plugins/libimage-reaction/bin/64bit
cp libimage-reaction.so ~/.config/obs-studio/plugins/libimage-reaction/bin/64bit/
```

## Building for Windows from GNU/Linux:
You need to download MinGW, OBS Studio source code, Wine, install OBS Studio using wine.
```
git clone https://github.com/scaledteam/obs-image-reaction
cd obs-image-reaction
mkdir build-win
cd build-win
cmake ..  -DCMAKE_SYSTEM_NAME=Windows  -DCMAKE_CXX_COMPILER=/usr/bin/x86_64-w64-mingw32-g++-win32  -DCMAKE_C_COMPILER=/usr/bin/x86_64-w64-mingw32-gcc-win32 -DLIBOBS_INCLUDE_DIR=~/git/obs-studio-27.0.1/libobs -DLIBOBS_LIB=~/.wine/drive_c/Program\ Files/obs-studio/bin/64bit/obs.dll
make
```
Now move libimage-reaction.dll into OBS Plugin directory.
