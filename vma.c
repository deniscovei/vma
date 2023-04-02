#include <stdlib.h>
#include <string.h>
#include "vma.h"
#include "commands.h"

arena_t *alloc_arena(const size_t size)
{
	arena_t *new_arena = malloc(sizeof(arena_t));
	CHECK_MEMORY(new_arena, MALLOC_FAILURE);
	new_arena->arena_size = size;
	new_arena->free_size = size;
	new_arena->block_list = init_list(sizeof(block_t));
	return new_arena;
}

void clear_miniblock(void *data)
{
	miniblock_t *miniblock = (miniblock_t *)data;
	free(miniblock->rw_buffer);
	free(miniblock);
}

void clear_block(void *data)
{
	block_t *block = (block_t *)data;
	clear(block->miniblock_list, clear_miniblock);
	free(block);
}

void dealloc_arena(arena_t *arena)
{
	clear(arena->block_list, clear_block);
	free(arena);
}

bool is_inside(add_t middle_address, add_t left_address, add_t right_address)
{
	return left_address <= middle_address &&
		middle_address < right_address;
}

perm_t set_permissions(const char perm_string[])
{
	perm_t mask = 0;
	mask += (strchr(perm_string, 'R') ? 4 : 0);
	mask += (strchr(perm_string, 'W') ? 2 : 0);
	mask += (strchr(perm_string, 'X') ? 1 : 0);
	return mask;
}

void get_permissions(char perm_string[], const perm_t perm)
{
	CHECK_MEMORY(perm_string, MALLOC_FAILURE);
	perm_string[0] = (perm & 4 ? 'R' : '-');
	perm_string[1] = (perm & 2 ? 'W' : '-');
	perm_string[2] = (perm & 1 ? 'X' : '-');
	perm_string[3] = '\0';
}

miniblock_t init_miniblock(const add_t address, const size_t size)
{
	miniblock_t new_miniblock;

	// set address
	new_miniblock.start_address = address;

	// set size
	new_miniblock.size = size;

	// set permissions
	new_miniblock.perm = set_permissions(DEFAULT_PERMISSIONS);

	// set rw_buffer
	new_miniblock.rw_buffer = calloc(size, sizeof(char));

	return new_miniblock;
}

block_t init_block(const add_t address, const size_t size)
{
	block_t new_block;

	// set address
	new_block.start_address = address;

	// set size
	new_block.size = size;

	// initialise miniblock list
	new_block.miniblock_list = init_list(sizeof(miniblock_t));

	return new_block;
}

void alloc_block(arena_t *arena, const add_t address, const size_t size)
{
	if (address >= arena->arena_size) {
		DISPLAY_ERROR(INVALID_START_ADDRESS);
		return;
	}

	if (address + size > arena->arena_size) {
		DISPLAY_ERROR(INVALID_END_ADDRESS);
		return;
	}

	node_t *prev_block_node = NULL;
	node_t *next_block_node = NULL;
	list_t *block_list = arena->block_list;

	// iterate through blocks list
	for (node_t *it = block_list->begin; it; it = it->next) {
		block_t *block = (block_t *)it->data;

		// check for possible overlaps between blocks
		if (is_inside(address, block->start_address,
					  block->start_address + block->size) ||
			is_inside(address + size - 1, block->start_address,
					  block->start_address + block->size) ||
			is_inside(block->start_address, address, address + size)) {
			DISPLAY_ERROR(INVALID_ALLOC_BLOCK);
			return;
		}

		// check if the leftmost block after the new block is found
		if (address + size <= block->start_address) {
			next_block_node = it;
			break;
		}

		prev_block_node = it;
	}

	arena->free_size -= size;

	// initialise a new miniblock
	miniblock_t new_miniblock = init_miniblock(address, size);

	block_t *prev_block = NULL;
	block_t *next_block = NULL;
	bool merge_prev = false;
	bool merge_next = false;

	// check if previous block can be merged with the new block
	if (prev_block_node) {
		prev_block = (block_t *)prev_block_node->data;
		merge_prev = (prev_block->start_address + prev_block->size == address);
	}

	// check if next block can be merged with the new block
	if (next_block_node) {
		next_block = (block_t *)next_block_node->data;
		merge_next = (address + size == next_block->start_address);
	}

	if (merge_prev) {
		push_back(prev_block->miniblock_list, &new_miniblock);
		if (merge_next) {
			merge(prev_block->miniblock_list, next_block->miniblock_list);
			prev_block->size += next_block->size;

			/* erase the next block node from arena block list without
				clearing its data, in order to keep it in the merged block */
			erase(arena->block_list, next_block_node, free);
		}
		prev_block->size += size;
	} else if (merge_next) {
		push_front(next_block->miniblock_list, &new_miniblock);
		next_block->start_address = address;
		next_block->size += size;
	} else {
		block_t new_block = init_block(address, size);
		push_back(new_block.miniblock_list, &new_miniblock);
		insert(arena->block_list, next_block_node, &new_block);
	}
}

metadata_t search_address(arena_t *arena, add_t address)
{
	list_t *block_list = arena->block_list;
	size_t miniblock_no = 0;

	// iterate through blocks list
	for (node_t *it = block_list->begin; it; it = it->next) {
		block_t *block = (block_t *)it->data;
		list_t *miniblock_list = block->miniblock_list;
		miniblock_no = 0;

		// iterate through miniblocks list
		for (node_t *jt = miniblock_list->begin; jt; jt = jt->next) {
			miniblock_t *miniblock = (miniblock_t *)jt->data;
			miniblock_no++;

			// check if a corresponding miniblock is found
			if (miniblock->start_address <= address &&
				address < miniblock->start_address + miniblock->size) {
				return (metadata_t) { it, jt, miniblock_no };
			}
		}
	}

	return (metadata_t) { NULL, NULL, 0 };
}

void free_block(arena_t *arena, const add_t address)
{
	metadata_t metadata = search_address(arena, address);

	node_t *block_node = metadata.block_node;
	node_t *miniblock_node = metadata.miniblock_node;
	size_t miniblock_no = metadata.miniblock_no;

	miniblock_t *miniblock = NULL;
	bool miniblock_found = false;

	if (miniblock_node) {
		miniblock = (miniblock_t *)miniblock_node->data;
		miniblock_found = (miniblock->start_address == address);
	}

	if (!miniblock_found) {
		DISPLAY_ERROR(INVALID_ADDRESS_FREE);
		return;
	}

	block_t *block = (block_t *)block_node->data;

	arena->free_size += miniblock->size;

	if (miniblock_node == block->miniblock_list->end) {
		block->size -= miniblock->size;
		pop_back(block->miniblock_list, clear_miniblock);

		if (empty(block->miniblock_list))
			erase(arena->block_list, block_node, clear_block);
	} else if (miniblock_node == block->miniblock_list->begin) {
		block->size -= miniblock->size;
		pop_front(block->miniblock_list, clear_miniblock);

		if (empty(block->miniblock_list))
			erase(arena->block_list, block_node, clear_block);
	} else {
		block_t new_block = init_block(address + miniblock->size,
			block->start_address + block->size - address - miniblock->size);

		assign_members(new_block.miniblock_list, miniblock_node->next,
					   block->miniblock_list->end,
					   block->miniblock_list->size - miniblock_no);

		assign_members(block->miniblock_list, block->miniblock_list->begin,
					   miniblock_node, miniblock_no);

		block->size = address - block->start_address;
		erase(block->miniblock_list, miniblock_node, clear_miniblock);
		insert(arena->block_list, block_node->next, &new_block);
	}
}

miniblock_t *find_miniblock(arena_t *arena, add_t address)
{
	list_t *block_list = arena->block_list;

	// iterate through blocks list
	for (node_t *it = block_list->begin; it; it = it->next) {
		block_t *block = (block_t *)it->data;
		list_t *miniblock_list = block->miniblock_list;

		// iterate through miniblocks list
		for (node_t *jt = miniblock_list->begin; jt; jt = jt->next) {
			miniblock_t *miniblock = (miniblock_t *)jt->data;

			// check if a corresponding miniblock is found
			if (miniblock->start_address == address)
				return miniblock;
		}
	}

	return NULL;
}

size_t length(void *ptr, size_t size)
{
	size_t len = 0;
	char *ch_ptr = (char *)ptr;

	while (len < size) {
		if (*(ch_ptr + len) == '\0')
			return len;
		len++;
	}

	return len;
}

void read(arena_t *arena, add_t address, size_t size)
{
	metadata_t metadata = search_address(arena, address);
	bool miniblock_found = metadata.block_node;

	if (!miniblock_found) {
		DISPLAY_ERROR(INVALID_ADDRESS_READ);
		return;
	}

	miniblock_t *first_miniblock = (miniblock_t *)metadata.miniblock_node->data;
	size_t max_read_size = first_miniblock->start_address +
					   first_miniblock->size - address;
	size_t address_jump = max_read_size;

	// check permissions of first block
	if (!(first_miniblock->perm & PROT_READ)) {
		DISPLAY_ERROR(INVALID_PERMISSIONS_READ);
		return;
	}

	// check write permissions of all the blocks we want to write in
	for (node_t *it = metadata.miniblock_node->next; it; it = it->next) {
		miniblock_t *miniblock = (miniblock_t *)it->data;

		// check permissions
		if (!(miniblock->perm & PROT_READ)) {
			DISPLAY_ERROR(INVALID_PERMISSIONS_READ);
			return;
		}

		max_read_size += miniblock->size;
	}

	if (max_read_size < size)
		DISPLAY_WARNING(WARNING_READ_SIZE, max_read_size);

	size_t read_size = min(max_read_size, size);

	fwrite(first_miniblock->rw_buffer + address -
		   first_miniblock->start_address, sizeof(char),
		   min(min(read_size, first_miniblock->start_address +
		   first_miniblock->size - address),
		   length(first_miniblock->rw_buffer + address -
		   first_miniblock->start_address, first_miniblock->start_address +
		   first_miniblock->size - address)), stdout);

	// copy data in available miniblocks
	for (node_t *it = metadata.miniblock_node->next; it; it = it->next) {
		if (address_jump >= read_size) {
			printf("\n");
			return;
		}

		miniblock_t *miniblock = (miniblock_t *)it->data;
		fwrite(miniblock->rw_buffer, sizeof(char),
			   min(min(miniblock->size, read_size - address_jump),
				   length(miniblock->rw_buffer, miniblock->size)), stdout);
		address_jump += miniblock->size;
	}

	printf("\n");
}

void write(arena_t *arena, const add_t address, const size_t size, void *data)
{
	metadata_t metadata = search_address(arena, address);
	bool miniblock_found = metadata.block_node;

	if (!miniblock_found) {
		DISPLAY_ERROR(INVALID_ADDRESS_WRITE);
		return;
	}

	miniblock_t *first_miniblock = (miniblock_t *)metadata.miniblock_node->data;
	size_t max_write_size = first_miniblock->start_address +
						first_miniblock->size - address;
	size_t address_jump = max_write_size;

	// check permissions of first block
	if (!(first_miniblock->perm & PROT_WRITE)) {
		DISPLAY_ERROR(INVALID_PERMISSIONS_WRITE);
		return;
	}

	// check write permissions of all blocks write is needed
	for (node_t *it = metadata.miniblock_node->next; it; it = it->next) {
		miniblock_t *miniblock = (miniblock_t *)it->data;

		// check permissions
		if (!(miniblock->perm & PROT_WRITE)) {
			DISPLAY_ERROR(INVALID_PERMISSIONS_WRITE);
			return;
		}

		max_write_size += miniblock->size;
	}

	size_t write_size = min(max_write_size, size);

	if (max_write_size < size)
		DISPLAY_WARNING(WARNING_WRITE_SIZE, max_write_size);

	memcpy(first_miniblock->rw_buffer + address -
		   first_miniblock->start_address, data, min(write_size,
		   first_miniblock->start_address + first_miniblock->size - address));

	// copy data in available miniblocks
	for (node_t *it = metadata.miniblock_node->next; it; it = it->next) {
		if (address_jump >= write_size)
			return;

		miniblock_t *miniblock = (miniblock_t *)it->data;
		memcpy(miniblock->rw_buffer, data + address_jump,
			   min(miniblock->size, write_size - address_jump));
		address_jump += miniblock->size;
	}
}

void pmap(const arena_t *arena)
{
	size_t total_memory = arena->arena_size;
	printf("Total memory: 0x%lX bytes\n", total_memory);

	size_t free_memory = arena->free_size;
	printf("Free memory: 0x%lX bytes\n", free_memory);

	size_t no_blocks = arena->block_list->size;
	printf("Number of allocated blocks: %ld\n", no_blocks);

	size_t no_miniblocks = 0;
	list_t *block_list = arena->block_list;

	// iterate through blocks list
	for (node_t *it = block_list->begin; it; it = it->next)
		no_miniblocks += ((block_t *)it->data)->miniblock_list->size;

	printf("Number of allocated miniblocks: %ld\n", no_miniblocks);
	size_t block_count = 0;

	char perm_string[PERM_SIZE];

	// iterate through blocks list
	for (node_t *it = block_list->begin; it; it = it->next) {
		block_t *block = (block_t *)it->data;
		printf("\nBlock %ld begin\n", ++block_count);
		printf("Zone: 0x%lX - 0x%lX\n", block->start_address,
			   block->start_address + block->size);

		list_t *miniblock_list = block->miniblock_list;
		size_t miniblock_count = 0;

		// iterate through miniblocks list
		for (node_t *jt = miniblock_list->begin; jt; jt = jt->next) {
			miniblock_t *miniblock = (miniblock_t *)jt->data;
			get_permissions(perm_string, (miniblock)->perm);
			printf("Miniblock %ld:\t\t0x%lX\t\t-\t\t0x%lX\t\t| %s\n",
				   ++miniblock_count, miniblock->start_address,
				   miniblock->start_address + miniblock->size, perm_string);
		}

		printf("Block %ld end\n", block_count);
	}
}

perm_t get_mask(char perm_keys_merged[])
{
	char perm_key[BUFF_SIZE];
	perm_t perm = 0;
	size_t lenght = 0;

	perm_keys_merged[strlen(perm_keys_merged) - 1] = '\0';

	char *curr_ptr = perm_keys_merged;
	char *next_ptr;
	char *cpy = NULL;
	while (curr_ptr) {
		next_ptr = strchr(curr_ptr + 1, '|');

		cpy = curr_ptr + (curr_ptr[0] == '|' ? 2 : 1);
		lenght = (next_ptr ? (size_t)(next_ptr - cpy - 1) :
				  strlen((curr_ptr)));
		strncpy(perm_key, cpy, lenght);
		perm_key[lenght] = '\0';

		// add new permission using its hash value
		perm += hash(perm_key);

		curr_ptr = next_ptr;
	}

	return perm;
}

void mprotect(arena_t *arena, add_t address, perm_t permission)
{
	miniblock_t *miniblock = find_miniblock(arena, address);

	if (miniblock) {
		miniblock->perm = permission;
		return;
	}

	DISPLAY_ERROR(INVALID_ADDRESS_MPROTECT);
}
