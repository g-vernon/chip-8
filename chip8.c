#include "chip8.h"

#include "draw.h"
#include "input.h"

#include <SDL2/SDL.h>

#include <stdbool.h>

#define DISPLAY_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT)
#define FONTSET_SIZE 80
#define RAM_SIZE 4096 /* In bytes */
#define ROM_LOAD_ADDRESS 0x200
#define STACK_SIZE 16
#define NUM_GP_REGS 16

#define X(opCode) ((opCode & 0x0F00) >> 8)
#define Y(opCode) ((opCode & 0x00F0) >> 4)

struct App {
	SDL_Renderer *renderer;
	SDL_Window *window;
};

struct Chip8 {
	uint8_t v[NUM_GP_REGS];	    /* General purpose registers V0 to VF */
	uint16_t pc;		    /* Program counter */
	uint8_t sp;		    /* Stack pointer */
	uint16_t I;		    /* Index register */
	uint8_t opCode;		    /* Operation code */
	uint8_t ram[RAM_SIZE];	    /* 4KB RAM */
	uint16_t stack[STACK_SIZE]; /* Stack... */
	uint8_t delayTimer; /* Counts down at 60Hz until reaching zero */
	uint8_t soundTimer; /* Counts down at 60Hz until reaching zero */
	bool display[DISPLAY_WIDTH][DISPLAY_HEIGHT];
	bool displayDirty;   /* Control flag */
	bool clearDisplay;   /* Control flag */
	Keyboard_t keyboard; /* Map of keypresses */
};

uint8_t chip8Fontset[FONTSET_SIZE] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
	0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
	0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
	0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
	0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
	0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
	0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
	0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
	0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
	0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
	0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
	0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
	0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
	0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
	0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
	0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

void SdlError(char *fmt, ...)
{
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

void InitSdl(struct App *app)
{
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

void InitChip8(struct Chip8 *chip8)
{
	/* Initialise registers and memory */
	chip8->pc = 0x200;
	chip8->opCode = 0;
	chip8->I = 0;
	chip8->sp = 0;

	/* Clear display */
	memset(chip8->display, 0, DISPLAY_SIZE * sizeof(chip8->display[0][0]));

	/* Zero stack */
	memset(chip8->stack, 0, STACK_SIZE * sizeof(chip8->stack[0]));

	/* Zero registers V0-VF */
	memset(chip8->v, 0, NUM_GP_REGS * sizeof(chip8->v[0]));

	/* Zero RAM */
	memset(chip8->ram, 0, RAM_SIZE * sizeof(chip8->ram[0]));

	/* Load fontset into RAM */
	for (int i = 0; i < FONTSET_SIZE; ++i)
		chip8->ram[i] = chip8Fontset[i];

	/* Reset timers */
	chip8->soundTimer = 0;
	chip8->delayTimer = 0;

	/* Reset control flags */
	chip8->displayDirty = false;
	chip8->clearDisplay = false;
}

void EmulateCycle(struct Chip8 *chip8)
{
	/* Fetch */
	uint16_t opCode =
		(chip8->ram[chip8->pc] << 8) | chip8->ram[chip8->pc + 1];

	/* Decode & execute */
	int16_t temp; /* Use to detect overflow in uint8_t arithmetic */
	uint8_t overflow;
	switch (opCode & 0xF000) {
	case 0x0000:
		/* Not fully implementing arch-specific subroutines */
		switch (opCode) {
		case 0x00E0:
			chip8->clearDisplay = true;
			chip8->pc += 2;
			break;
		case 0x00EE:
			chip8->sp -= 1;
			chip8->pc = chip8->stack[chip8->sp];
			break;
		default:
			fprintf(stderr, "Invalid opCode %#X\n", opCode);
			exit(1);
		}
		break;
	case 0x1000: /* 1NNN: Jump to NNN */
		chip8->pc = opCode & 0x0FFF;
		break;
	case 0x2000: /* 2NNN: Call subroutine at NNN */
		chip8->stack[chip8->sp] = chip8->pc + 2;
		chip8->sp += 1;
		chip8->pc = opCode & 0x0FFF;
		break;
	case 0x3000: /* 3XNN: Skip next instruction if VX == NN */
		if (chip8->v[X(opCode)] == (opCode & 0x00FF)) {
			chip8->pc += 4;
		} else {
			chip8->pc += 2;
		}
		break;
	case 0x4000: /* 4XNN: Skip next instruction if VX != NN */
		if (chip8->v[X(opCode)] != (opCode & 0x00FF)) {
			chip8->pc += 4;
		} else {
			chip8->pc += 2;
		}
		break;
	case 0x5000: /* 5XY0: Skip next instruction if VX == VY */
		if (chip8->v[X(opCode)] == chip8->v[Y(opCode)]) {
			chip8->pc += 4;
		} else {
			chip8->pc += 2;
		}
		break;
	case 0x6000: /* 6XNN: Set VX to NN */
		chip8->v[X(opCode)] = opCode & 0x00FF;
		chip8->pc += 2;
		break;
	case 0x7000: /* 7XNN: Add NN to VX (do not change carry flag) */
		chip8->v[X(opCode)] += opCode & 0x00FF;
		chip8->pc += 2;
		break;
	case 0x8000: /* ALU operations */
		switch (opCode & 0xF00F) {
		case 0x8000: /* 8XY0: Set VX = VY */
			chip8->v[X(opCode)] = chip8->v[Y(opCode)];
			chip8->pc += 2;
			break;
		case 0x8001: /* 8XY1: Set VX = VX | VY */
			chip8->v[X(opCode)] |= chip8->v[Y(opCode)];
			chip8->pc += 2;
			break;
		case 0x8002: /* 8XY2: Set VX = VX & VY */
			chip8->v[X(opCode)] &= chip8->v[Y(opCode)];
			chip8->pc += 2;
			break;
		case 0x8003: /* 8XY3: Set VX = VX ^= VY */
			chip8->v[X(opCode)] ^= chip8->v[Y(opCode)];
			chip8->pc += 2;
			break;
		case 0x8004: /* 8XY4: VX += VY. VF iff overflow. */
			temp = chip8->v[X(opCode)] + chip8->v[Y(opCode)];
			chip8->v[X(opCode)] = temp & 0xFF;
			chip8->v[0xF] = (temp > 0xFF) ? 1 : 0;
			chip8->pc += 2;
			break;
		case 0x8005: /* 8XY5: VX -= VY. VF iff not underflow. */
			overflow = chip8->v[X(opCode)] >= chip8->v[Y(opCode)]
					   ? 1
					   : 0;
			chip8->v[X(opCode)] -= chip8->v[Y(opCode)];
			chip8->v[0xF] = overflow;
			chip8->pc += 2;
			break;
		case 0x8006: /* 8XY6: VX >>= 1. VF stores least
				significant bit prior to shift.
			      */
		{
			uint8_t lsb = chip8->v[X(opCode)] & 0x1;
			chip8->v[X(opCode)] >>= 1;
			chip8->v[0xF] = lsb;
			chip8->pc += 2;
			break;
		}
		case 0x8007: /* 8XY7: Set VX = VY - VX. VF is
				set to 0 iff underflow. */
		{
			uint8_t underflow =
				chip8->v[X(opCode)] > chip8->v[Y(opCode)];
			chip8->v[X(opCode)] =
				chip8->v[Y(opCode)] - chip8->v[X(opCode)];
			chip8->v[0xF] = !underflow;
			chip8->pc += 2;
			break;
		}
		case 0x800E: /* 8XYE: VX <<= 1. VF stores most
				significant bit of VX prior to
				shift. */
		{
			uint8_t msb = chip8->v[X(opCode)] & 0x80;
			chip8->v[X(opCode)] <<= 1;
			chip8->v[0xF] = msb ? 1 : 0;
			chip8->pc += 2;
			break;
		}
		default:
			fprintf(stderr, "Invalid opCode %#X\n", opCode);
			exit(1);
		}
		/* Unreachable */
		break;
	case 0x9000: /* 9XY0: Skip next instruction if VX != VY. */
		if (chip8->v[X(opCode)] != chip8->v[Y(opCode)]) {
			chip8->pc += 4;
		} else {
			chip8->pc += 2;
		}
		break;
	case 0xA000: /* ANNN: Set I to NNN */
		chip8->I = opCode & 0x0FFF;
		chip8->pc += 2;
		break;
	case 0xB000: /* BNNN: Jump to address NNN + V0 */
		chip8->pc = (opCode & 0x0FFF) + chip8->v[0];
		break;
	case 0xC000: /* CXNN: Set VX to result of a bitwise and
			operation on a random number & NN. */
		chip8->v[X(opCode)] = rand() & (opCode & 0x00FF);
		chip8->pc += 2;
		break;
	case 0xD000: /* DXYN: Draw a sprite at (VX, VY) with width 8 and
			height N pixels. */
	{
		int n = 0;
		int vx = chip8->v[X(opCode)];
		int vy = chip8->v[Y(opCode)];
		bool unsetScreenPixel = false;

		while (n < (opCode & 0xF)) {
			uint8_t rowData = chip8->ram[chip8->I + n];
			for (int i = 0; i < 8; i++) {
				if ((rowData << i) & 0x80u) {
					if (chip8->display[vx + i][vy + n])
						unsetScreenPixel = true;
					chip8->display[vx + i][vy + n] =
						!chip8->display[vx + i][vy + n];
				}
			}
			n++;
		}
		chip8->v[0xF] = unsetScreenPixel ? 1 : 0;
		chip8->pc += 2;
		chip8->displayDirty = true;
		break;
	}
	case 0xE000: /* Keyboard */
		switch (opCode & 0xF0FF) {
		case 0xE09E: /* EX9E: Skip the next instruction if the key
				stored in VX is pressed */
			if (chip8->keyboard.key[chip8->v[X(opCode)]]) {
				chip8->pc += 4;
			} else {
				chip8->pc += 2;
			}
			break;
		case 0xE0A1: /* EXA1: Skip the next instruction if the key
				stored in VX is not pressed */
			if (!chip8->keyboard.key[chip8->v[X(opCode)]]) {
				chip8->pc += 4;
			} else {
				chip8->pc += 2;
			}
			break;
		default:
			fprintf(stderr, "Unknown opcode %#X\n", opCode);
			exit(1);
			break;
		}
		break;
	case 0xF000: /* Peripherals */
		switch (opCode & 0xF0FF) {
		case 0xF007: /* FX07: Set VX to value of the delay timer */
			chip8->v[X(opCode)] = chip8->delayTimer;
			chip8->pc += 2;
			break;
		case 0xF00A: /* FX0A: Await keypress and store it in VX */
			/*
			 * This is a noddy implementation, if any key is already
			 * down it will not wait for a new keyress
			 */
			for (int i = 0; i <= 0xF; i++) {
				if (chip8->keyboard.key[i]) {
					chip8->v[X(opCode)] = i;
					chip8->pc += 2;
					break;
				}
			}
			/* If no key found, do not increment pc */
			break;
		case 0xF015: /* FX15: Set delay timer to VX */
			chip8->delayTimer = chip8->v[X(opCode)];
			chip8->pc += 2;
			break;
		case 0xF018: /* FX18: Set sound timer to VX */
			chip8->soundTimer = chip8->v[X(opCode)];
			chip8->pc += 2;
			break;
		case 0xF01E: /* FX1E: Add VX to I. */
			chip8->I += chip8->v[X(opCode)];
			chip8->pc += 2;
			break;
		case 0xF029: /* FX29: Set I to the location of the sprite for
				the character in VX */
			chip8->I = 5 * chip8->v[X(opCode)];
			chip8->pc += 2;
			break;
		case 0xF033: /*
			      * FX33: Store the BCD representation of VX
			      * Hundreds digit in memory at location in I
			      * Tens digit at location I + 1
			      * Ones digit at location I + 2
			      */
			chip8->ram[chip8->I] = chip8->v[X(opCode)] / 100;
			chip8->ram[chip8->I + 1] =
				(chip8->v[X(opCode)] % 100) / 10;
			chip8->ram[chip8->I + 2] = chip8->v[X(opCode)] % 10;
			chip8->pc += 2;
			break;
		case 0xF055: /* FX33: Store V0 to VX inclusive starting at
				address I */
			for (int i = 0; i <= X(opCode); i++)
				chip8->ram[chip8->I + i] = chip8->v[i];
			chip8->pc += 2;
			break;
		case 0xF065: /* FX65: Fill V0 to VX inclusive with values from
				memory starting at address I */
			for (int i = 0; i <= X(opCode); i++)
				chip8->v[i] = chip8->ram[chip8->I + i];
			chip8->pc += 2;
			break;
		default:
			fprintf(stderr, "Unknown opcode %#X\n", opCode);
			exit(1);
			break;
		}
		break;
	default:
		fprintf(stderr, "Unknown opcode %#X\n", opCode);
		exit(1);
		break;
	}

	/*
	 * Update timers
	 * TODO: They should count down at 60Hz, currently timer clock speed is
	 * same as CPU clock speed.
	 */
	if (chip8->delayTimer > 0) chip8->delayTimer -= 1;

	if (chip8->soundTimer > 0) {
		chip8->soundTimer -= 1;
		if (chip8->soundTimer == 0) printf("BEEP\n");
	}
}

void LoadRom(uint8_t *loadAddress, const char *const fileName)
{
	FILE *Rom = fopen(fileName, "r");
	int i = 0;
	int c;

	if (Rom == NULL) {
		perror("Failed to open ROM");
		exit(EXIT_FAILURE);
	}

	while ((c = getc(Rom)) != EOF)
		loadAddress[i++] = c;

	fclose(Rom);
}

void Usage()
{
	printf("./chip8 <filepath>\n");
	printf("- filepath: File path to ROM to load\n");
}

int main(int argc, char *argv[])
{
	struct App app;
	struct Chip8 chip8;

	if (argc != 2) {
		Usage();
		exit(EXIT_FAILURE);
	}

	InitSdl(&app);
	InitChip8(&chip8);
	/* TODO: Sanitise input */
	LoadRom(chip8.ram + ROM_LOAD_ADDRESS, argv[1]);

	PrepareDisplay(app.renderer);
	while (1) {
		EmulateCycle(&chip8);
		DoInput(&chip8.keyboard);
		if (chip8.displayDirty) {
			DrawDisplay(chip8.display);
			chip8.displayDirty = false;
		}
		if (chip8.clearDisplay) {
			PrepareDisplay(app.renderer);
			chip8.clearDisplay = false;
		}
		PresentDisplay(app.renderer);
		SDL_Delay(1); /* 1ms delay */
	}
	return 0;
}
