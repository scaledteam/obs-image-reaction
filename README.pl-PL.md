[English](README.md) | [Polski](README.pl-PL.md) | [繁體中文](README.zh-TW.md)

# Wtyczka OBS Image Reaction
Obraz, który reaguje na źródło dźwięku.

## Instalowanie plików binarnych
Pobierz pliki binarne z [oficjalnych wydań](https://github.com/scaledteam/obs-image-reaction/releases/) lub wersję bardziej przyjazną dla użytkownika od [ashmanix](https://github.com/ashmanix/obs-image-reaction/releases).

Dla systemów Windows, przenieś zawartość wtyczki do katalogu, w którym znajduje się instalacja OBS'a. Zazwyczaj jest to "C:\Program Files\obs-studio\".

Dla systemów GNU/Linux wrzuć folder "libimage-reaction" do folderu "~/.config/obs-studio/plugins/".

Dla Mac OS wypróbuj ten port od ashmanix: https://github.com/ashmanix/obs-image-reaction.

## Kompilowanie i instalacja na systemach GNU/Linux:
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

## Kompilowanie pod system Windows z systemu GNU/Linux:
Musisz pobrać MinGW, kod źródłowy OBS Studio, Wine, zainstaluj OBS Studio przy użyciu Wine.
```
git clone https://github.com/scaledteam/obs-image-reaction
cd obs-image-reaction
mkdir build-win
cd build-win
cmake ..  -DCMAKE_SYSTEM_NAME=Windows  -DCMAKE_CXX_COMPILER=/usr/bin/x86_64-w64-mingw32-g++-win32  -DCMAKE_C_COMPILER=/usr/bin/x86_64-w64-mingw32-gcc-win32 -DLIBOBS_INCLUDE_DIR=~/git/obs-studio-27.0.1/libobs -DLIBOBS_LIB=~/.wine/drive_c/Program\ Files/obs-studio/bin/64bit/obs.dll
make
```
A potem przenieś plik libimage-reaction.dll do katalogu wtyczek OBS.
