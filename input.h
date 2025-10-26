#ifndef INPUT_H_
#define INPUT_H_

#include <stdbool.h>

typedef union Keyboard {
	struct {
		bool key0;
		bool key1;
		bool key2;
		bool key3;
		bool key4;
		bool key5;
		bool key6;
		bool key7;
		bool key8;
		bool key9;
		bool keyA;
		bool keyB;
		bool keyC;
		bool keyD;
		bool keyE;
		bool keyF;
	};
	bool key[16];
} Keyboard_t;

void DoInput(Keyboard_t *keyboard);

#endif /* INPUT_H_ */
