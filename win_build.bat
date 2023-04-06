@echo off

set path=C:\SDL2\lib\x64;%path%
set SDL_AUDIODRIVER=dsound

if not exist "build\" mkdir build\
cd build/

:: set FLAGS=/EHsc /std:c11 
set FLAGS=/W4 /WX /wd4996 /wd5105 /FC /TC /Zi /nologo /std:c11
set INCLUDES=/I ..\src /I C:\SDL2\include 
set SRC_FILES= ..\src\sdl_fours.c 
set SDL_PATH=C:\SDL2\lib\x64\
set LIBS=user32.lib shell32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib 

cl %FLAGS% %INCLUDES% %SRC_FILES% /link /LIBPATH:%SDL_PATH% %LIBS% /SUBSYSTEM:CONSOLE

cd ..
