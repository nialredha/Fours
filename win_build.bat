@echo off

set path=C:\SDL2\lib\x64;%path%
set SDL_AUDIODRIVER=dsound

if not exist "build\" mkdir build\
cd build/

:: set FLAGS=/EHsc
set FLAGS=/W4 /WX /std:c11 /wd4996 /wd5105 /FC /TC /Zi /nologo
set INCLUDES=/I ..\src /I C:\SDL2\include 
set SDL_PATH=C:\SDL2\lib\x64\
set LIBS=user32.lib shell32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib 

cl %FLAGS% %INCLUDES% ..\src\main.c ..\src\graphics.c /link /LIBPATH:%SDL_PATH% %LIBS% /SUBSYSTEM:CONSOLE

cd ..
