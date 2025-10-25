#include <SDL2/SDL.h>

void prepareScene(SDL_Renderer *renderer)
{
	SDL_SetRenderDrawColor(renderer, 96, 128, 255, 255);
	SDL_RenderClear(renderer);
}

void presentScene(SDL_Renderer *renderer)
{
	SDL_RenderPresent(renderer);
}
