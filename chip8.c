#include "input.h"
#include "draw.h"

#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

struct App {
  SDL_Renderer *renderer;
  SDL_Window *window;
};

void SdlError(char *fmt, ...) {
  const int MaxSize = 50;
  char msg[MaxSize];
  va_list args;

  va_start(args, fmt);
  if (vsnprintf(msg, MaxSize, fmt, args) < 0) {
    fprintf(stderr, "Failed to write error\n");
    return;
  }
  va_end(args);

  fprintf(stderr, "%s: %s", msg, SDL_GetError());
}

void initSdl(struct App *app) {
  unsigned int rendererFlags = SDL_RENDERER_ACCELERATED;
  unsigned int windowFlags = 0;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SdlError("Couldn't initialise SDL");
    exit(1);
  }

  app->window = SDL_CreateWindow("Chip-8 Emu", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                SCREEN_HEIGHT, windowFlags);
  if (app->window == NULL) {
    SdlError("Failed to open window");
    exit(1);
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

  app->renderer = SDL_CreateRenderer(app->window, -1, rendererFlags);

  if (app->renderer == NULL) {
    SdlError("Failed to create renderer");
    exit(1);
  }
}

int main()
{
	struct App app;

	initSdl(&app);

	while (1) {
		prepareScene(app.renderer);
		doInput();
		presentScene(app.renderer);
		SDL_Delay(16);
	}
	return 0;
}
