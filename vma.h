#pragma once
#include "list.h"

#define PERM_NO 3
#define PERM_SIZE 4
#define DEFAULT_PERMISSIONS "RW-"
#define min(a, b) \
	({ __typeof__(a) _a = (a); \
	__typeof__(b) _b = (b); \
	_a < _b ? _a : _b; })

typedef unsigned long add_t;
typedef unsigned char perm_t;

typedef struct {
	add_t start_address;
	size_t size;
	list_t *miniblock_list;
} block_t;

typedef struct {
	add_t start_address;
	size_t size;
	perm_t perm;
	void *rw_buffer;
} miniblock_t;

typedef struct {
	node_t *block_node;
	node_t *miniblock_node;
	size_t miniblock_no;
} metadata_t;

typedef struct {
	add_t arena_size;
	add_t free_size;
	list_t *block_list;
} arena_t;

arena_t *alloc_arena(const size_t size);
void clear_miniblock(void *data);
void clear_block(void *data);
void dealloc_arena(arena_t *arena);

bool is_inside(add_t middle_address, add_t left_address, add_t right_address);
perm_t set_permissions(const char perm_string[]);
void get_permissions(char perm_string[], const perm_t perm);

miniblock_t init_miniblock(const add_t address, const size_t size);
block_t init_block(const add_t address, const size_t size);
void alloc_block(arena_t *arena, const add_t address, const size_t size);
metadata_t search_address(arena_t *arena, add_t address);

void free_block(arena_t *arena, const add_t address);
miniblock_t *find_miniblock(arena_t *arena, add_t address);

void read(arena_t *arena, add_t address, size_t size);
void write(arena_t *arena, const add_t address, const size_t size, void *data);
perm_t get_mask(char perm_keys_merged[]);
void pmap(const arena_t *arena);
void mprotect(arena_t *arena, add_t address, perm_t permission);
