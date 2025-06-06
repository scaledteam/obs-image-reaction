[English](README.md) | [繁體中文](README.zh-TW.md)

# OBS Image Reaction Plugin
Image that reacts to sound source.

**Note:** This is a temporary test build with significant new features. It may not be stable or performant.

## New Feature: Video Playback Support
This version includes a major refactor of the plugin's core logic. Instead of only displaying static images and GIFs, it now utilizes OBS's internal media source framework (`ffmpeg_source`). This change allows for the playback of video files (e.g., `.mov`, `.mp4`) as reactions.

### Key Changes:
- **Video Support:** The plugin can now load and play any video format supported by OBS's Media Source.
- **Refactored Core:** The image loading library has been completely replaced with media source handling. This may affect performance and stability.
- **Wider Format Support:** The file dialog now includes common video file formats.

## Installing binaries
Download binaries from [official releases](https://github.com/scaledteam/obs-image-reaction/releases/) or more user-friendly version from [ashmanix](https://github.com/ashmanix/obs-image-reaction/releases).

For Windows, Move the contents of plugin into your obs installation directory. It usually installed into "C:\Program Files\obs-studio\".

For GNU/Linux, put "libimage-reaction" folder into "~/.config/obs-studio/plugins/" folder.

For Mac OS, try this port from ashmanix: https://github.com/ashmanix/obs-image-reaction .

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
cp -r ../data  ~/.config/obs-studio/plugins/libimage-reaction/
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
