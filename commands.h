#pragma once
#include <stdio.h>

#define BUFF_SIZE 409600

#define CHECK_ARGS(check_endl, arg_cnt, arg_ref, instr) \
	do { \
		if (check_endl) { \
			char next_ch = getchar(); \
			if (next_ch == ' ') { \
				ungetc(next_ch, stdin); \
				DISPLAY_ERROR(INVALID_COMMAND); \
				break; \
			} \
		} \
		else if ((arg_cnt) != (arg_ref)) { \
			break; \
		} \
		instr; \
	} while (0)

enum keys {
	BADKEY, PROT_NONE = 0, PROT_EXEC = 1, PROT_WRITE = 2, PROT_READ = 4,
	ALLOC_ARENA, DEALLOC_ARENA, ALLOC_BLOCK, FREE_BLOCK, READ, WRITE, PMAP,
	MPROTECT
};

typedef struct set_t {
	char *key;
	int value;
} set_t;

// hash funciton for storing commands' names
int hash(char *key);

void get_command(char command[]);

void ignore_buffer(char *data, size_t size);
