#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "vma.h"

int main(void)
{
	arena_t *arena = NULL;

	while (1) {
		char command[BUFF_SIZE];
		char perm_keys_merged[BUFF_SIZE];
		char data[BUFF_SIZE];
		perm_t perm;
		add_t address;
		size_t size;
		int arg_count = 0;

		get_command(command);

		switch (hash(command)) {
		case ALLOC_ARENA:
			arg_count = scanf("%ld", &size);
			CHECK_ARGS(true, arg_count, 1, arena = alloc_arena(size));
			break;
		case ALLOC_BLOCK:
			arg_count = scanf("%ld%ld", &address, &size);
			CHECK_ARGS(true, arg_count, 2, alloc_block(arena, address, size));
			break;
		case DEALLOC_ARENA:
			CHECK_ARGS(true, arg_count, 0, dealloc_arena(arena));
			return 0;
		case FREE_BLOCK:
			arg_count = scanf("%ld", &address);
			CHECK_ARGS(true, arg_count, 1, free_block(arena, address));
			break;
		case PMAP:
			CHECK_ARGS(true, arg_count, 0, pmap(arena));
			break;
		case WRITE:
			arg_count = scanf("%ld%ld", &address, &size);
			fgetc(stdin);
			fread(data, sizeof(char), size, stdin);
			//ignore_buffer(data, size);
			CHECK_ARGS(false, arg_count, 2, write(arena, address, size, data));
			break;
		case READ:
			arg_count = scanf("%ld%ld", &address, &size);
			CHECK_ARGS(true, arg_count, 2, read(arena, address, size));
			break;
		case MPROTECT:
			arg_count = scanf("%ld", &address);
			fgets(perm_keys_merged, BUFF_SIZE, stdin);
			perm = get_mask(perm_keys_merged);
			CHECK_ARGS(false, arg_count, 1, mprotect(arena, address, perm));
			break;
		default:
			DISPLAY_ERROR(INVALID_COMMAND);
		}
	}
	return 0;
}
