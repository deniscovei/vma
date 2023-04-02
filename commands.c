#include <stdio.h>
#include <string.h>
#include "commands.h"

int hash(char *key)
{
	if (!key)
		return BADKEY;

	set_t hash_table[] = {
		{"ALLOC_ARENA", ALLOC_ARENA}, {"DEALLOC_ARENA", DEALLOC_ARENA},
		{"ALLOC_BLOCK", ALLOC_BLOCK}, {"FREE_BLOCK", FREE_BLOCK},
		{"READ", READ}, {"WRITE", WRITE}, {"PMAP", PMAP},
		{"MPROTECT", MPROTECT}, {"PROT_NONE", PROT_NONE},
		{"PROT_READ", PROT_READ}, {"PROT_WRITE", PROT_WRITE},
		{"PROT_EXEC", PROT_EXEC}
	};

	int count = sizeof(hash_table) / sizeof(set_t);

	for (int i = 0; i < count; i++) {
		set_t curr = hash_table[i];

		if (strcmp(curr.key, key) == 0)
			return curr.value;
	}

	return BADKEY;
}

void get_command(char command[])
{
	scanf("%s", command);
}

void ignore_buffer(char *data, size_t size)
{
	char buff[BUFF_SIZE];

	if (data[size - 1] != '\n') {
		char next_char = getchar();

		if (next_char != '\n')
			fgets(buff, BUFF_SIZE, stdin);
		else
			ungetc(next_char, stdin);
	}
}
