#ifndef DRAW_H_
#define DRAW_H_

#include "chip8.h"

#include <SDL2/SDL.h>

#include <stdbool.h>

void PrepareDisplay(SDL_Renderer *renderer);
void PresentDisplay(SDL_Renderer *renderer);
void DrawDisplay(bool display[DISPLAY_WIDTH][DISPLAY_HEIGHT]);

#endif /* DRAW_H_ */
