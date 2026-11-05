@echo off

mkdir ..\..\build
pushd ..\..\build
cl /std:c++17 -FC -Zi ..\HandmadeHeroRT\code\win32_handmade.cpp user32.lib Gdi32.lib

popd

pause