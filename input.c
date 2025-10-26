#include "input.h"

#include <SDL2/SDL.h>

void setKey(SDL_KeyboardEvent *event, Keyboard_t *keyboard, bool isDown)
{
	if (!event->repeat) {
		switch (event->keysym.scancode) {
		case SDL_SCANCODE_0:
			keyboard->key0 = isDown;
			break;
		case SDL_SCANCODE_1:
			keyboard->key1 = isDown;
			break;
		case SDL_SCANCODE_2:
			keyboard->key2 = isDown;
			break;
		case SDL_SCANCODE_3:
			keyboard->key3 = isDown;
			break;
		case SDL_SCANCODE_4:
			keyboard->key4 = isDown;
			break;
		case SDL_SCANCODE_5:
			keyboard->key5 = isDown;
			break;
		case SDL_SCANCODE_6:
			keyboard->key6 = isDown;
			break;
		case SDL_SCANCODE_7:
			keyboard->key7 = isDown;
			break;
		case SDL_SCANCODE_8:
			keyboard->key8 = isDown;
			break;
		case SDL_SCANCODE_9:
			keyboard->key9 = isDown;
			break;
		case SDL_SCANCODE_A:
			keyboard->keyA = isDown;
			break;
		case SDL_SCANCODE_B:
			keyboard->keyB = isDown;
			break;
		case SDL_SCANCODE_C:
			keyboard->keyC = isDown;
			break;
		case SDL_SCANCODE_D:
			keyboard->keyD = isDown;
			break;
		case SDL_SCANCODE_E:
			keyboard->keyE = isDown;
			break;
		case SDL_SCANCODE_F:
			keyboard->keyF = isDown;
			break;
		default:
			/* Don't care about other keys */
			break;
		}
	}
}

void DoInput(Keyboard_t *keyboard)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_KEYDOWN:
			setKey(&event.key, keyboard, true);
			break;
		case SDL_KEYUP:
			setKey(&event.key, keyboard, false);
			break;
		default:
			break;
		}
	}
}
