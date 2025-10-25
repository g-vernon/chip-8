CC = gcc
CFLAGS = -std=c17 -Wall -Werror -Wextra
SDL_FLAGS = $(shell sdl2-config --cflags --libs)

chip8: chip8.c input.c draw.c
	$(CC) $^ -o $@ $(CFLAGS) $(SDL_FLAGS)
