#include "draw.h"

#include "chip8.h"

#include <SDL2/SDL.h>

#include <stdbool.h>

SDL_Renderer *gRenderer = NULL;

const int scale = SCREEN_WIDTH / DISPLAY_WIDTH;

void ClearDisplay()
{
	SDL_SetRenderDrawColor(gRenderer, 50, 50, 60, 255);
	SDL_RenderClear(gRenderer);
	SDL_SetRenderDrawColor(gRenderer, 128, 255, 96, 255);
}

void PrepareDisplay(SDL_Renderer *renderer)
{
	gRenderer = renderer;
	ClearDisplay();
}

void PresentDisplay(SDL_Renderer *renderer)
{
	SDL_RenderPresent(renderer);
}

void DrawDisplay(bool display[DISPLAY_WIDTH][DISPLAY_HEIGHT])
{
	const int margin = 1;
	ClearDisplay();
	for (int i = 0; i < DISPLAY_WIDTH; i++) {
		for (int j = 0; j < DISPLAY_HEIGHT; j++) {
			if (display[i][j]) {
				SDL_Rect rect = {.x = scale * i + margin,
						 .y = scale * j + margin,
						 .w = scale - 2 * margin,
						 .h = scale - 2 * margin};
				SDL_RenderFillRect(gRenderer, &rect);
			}
		}
	}
}
