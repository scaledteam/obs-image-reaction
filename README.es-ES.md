[English](README.md) | [繁體中文](README.zh-TW.md) | [Español](README.es-ES.md)

# Plugin de Reacción de Imagen para OBS

Imagen que reacciona a la fuente de sonido.

## Instalación de binarios

Descarga los binarios desde las [versiones oficiales](https://github.com/scaledteam/obs-image-reaction/releases/) o una versión más fácil de usar de [ashmanix](https://github.com/ashmanix/obs-image-reaction/releases).

Para Windows, mueve el contenido del plugin a tu directorio de instalación de OBS. Normalmente está instalado en "C:\Program Files\obs-studio".

Para GNU/Linux, coloca la carpeta "libimage-reaction" en la carpeta "\~/.config/obs-studio/plugins/".

Para Mac OS, prueba este port de ashmanix: [https://github.com/ashmanix/obs-image-reaction](https://github.com/ashmanix/obs-image-reaction) .

## Compilar e instalar en GNU/Linux:

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

## Compilar para Windows desde GNU/Linux:

Necesitas descargar MinGW, el código fuente de OBS Studio, Wine, e instalar OBS Studio usando wine.

```
git clone https://github.com/scaledteam/obs-image-reaction
cd obs-image-reaction
mkdir build-win
cd build-win
cmake ..  -DCMAKE_SYSTEM_NAME=Windows  -DCMAKE_CXX_COMPILER=/usr/bin/x86_64-w64-mingw32-g++-win32  -DCMAKE_C_COMPILER=/usr/bin/x86_64-w64-mingw32-gcc-win32 -DLIBOBS_INCLUDE_DIR=~/git/obs-studio-27.0.1/libobs -DLIBOBS_LIB=~/.wine/drive_c/Program\ Files/obs-studio/bin/64bit/obs.dll
make
```

Ahora mueve libimage-reaction.dll al directorio de plugins de OBS.
