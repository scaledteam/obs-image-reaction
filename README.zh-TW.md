[English](README.md) | [繁體中文](README.zh-TW.md) | [Español](README.es-ES.md)

# OBS 聲控圖片插件
根據音訊來源變化顯示不同圖片的 OBS 插件。

## **安裝二進位檔**
可以從 [官方發布](https://github.com/scaledteam/obs-image-reaction/releases/) 下載插件，或下載由 [ashmanix](https://github.com/ashmanix/obs-image-reaction/releases) 提供的更易用版本。

### **Windows 安裝**
1. 下載插件並解壓縮。
2. 將插件內容移動到 OBS 的安裝目錄，通常位於 "C:\Program Files\obs-studio\"。
3. 重新啟動 OBS，並在來源清單中找到 **「聲控圖片」**。

### **GNU/Linux 安裝**
1. 下載插件並解壓縮。
2. 將 **libimage-reaction** 資料夾放入 "~/.config/obs-studio/plugins/"。
3. 重新啟動 OBS。

### **MacOS 安裝**
可以嘗試使用 ashmanix 提供的 macOS 版本： [https://github.com/ashmanix/obs-image-reaction](https://github.com/ashmanix/obs-image-reaction)。

## **在 GNU/Linux 上編譯與安裝：**
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

## 在 GNU/Linux 交叉編譯 Windows 版本：
如果你需要在 GNU/Linux 上編譯 Windows 版本，你需要下載 MinGW、OBS Studio 原始碼、Wine，並使用 Wine 安裝 OBS Studio。
```
git clone https://github.com/scaledteam/obs-image-reaction
cd obs-image-reaction
mkdir build-win
cd build-win
cmake ..  -DCMAKE_SYSTEM_NAME=Windows  -DCMAKE_CXX_COMPILER=/usr/bin/x86_64-w64-mingw32-g++-win32  -DCMAKE_C_COMPILER=/usr/bin/x86_64-w64-mingw32-gcc-win32 -DLIBOBS_INCLUDE_DIR=~/git/obs-studio-27.0.1/libobs -DLIBOBS_LIB=~/.wine/drive_c/Program\ Files/obs-studio/bin/64bit/obs.dll
make
```
編譯完成後，將 libimage-reaction.dll 移動到 OBS 插件目錄。
