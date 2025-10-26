# chip-8

## Overview
[CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) is an interpreted programming language and virtual machine specification.

This is an implementation in C using SDL2 as the platform layer.

## Building
Issue `make`.

## Usage
Issue `./chip8 <rom name>` e.g. `./chip-8 roms/pong.ch8`.

Keyboard input is via the hex keys 0-F.

## TODO
- Properly set clock speed for timers
- Wire up sound timer beep to SDL2 sound API
- Consider OR-ing multiple frames or slowly fading pixels out to help with flicker effects
