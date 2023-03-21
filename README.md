# Fours

a <u>s</u>imple <u>s</u>ixteen <u>s</u>tep <u>s</u>equencer offering 4/4 time

![Fours][./assets/Fours.png]

## Build in Windows

The following steps assume you have already setup MSVC in your Windows terminal:

1. Download [SDL2-devel-x.x.x-VC](https://github.com/libsdl-org/SDL/releases) and [SDL2\_ttf-devel-x.x.x-VC](https://github.com/libsdl-org/SDL_ttf/releases) to somewhere on your computer (e.g. C:\SDL2)
2. Move the SDL\_ttf header and library files to the same folder the SDL2 files are located.
3. Open `win_build.bat` in the base of this repo and change the SDL2 paths to match the location of SDL2 on your machine.
4. run `win_build.bat`

## Run in Windows

```
cd build
main.exe
```
