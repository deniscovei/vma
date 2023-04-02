#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "errors.h"

typedef struct node_t {
	void *data;
	struct node_t *prev, *next;
} node_t;

typedef struct list_t {
	node_t *begin, *end;
	size_t size, data_size;
} list_t;

/* ==========================================================================
   Errors
   ==========================================================================*/

// check if the list is initialised
bool check_list(list_t *list);

/* ==========================================================================
   Initialisers
   ==========================================================================*/

// initialise a list
list_t *init_list(size_t data_size);

// initialise a node
node_t *init_node(const void *data, const size_t data_size);

/* ==========================================================================
   Element access
   ==========================================================================*/

// access the first node of the list
node_t *front(list_t *list);

// access the last node of the list
node_t *back(list_t *list);

/* ==========================================================================
   Capacity
   ==========================================================================*/

// check if the list is empty
bool empty(list_t *list);

// get the size of the list
size_t size(list_t *list);

/* ==========================================================================
   Modifiers
   ==========================================================================*/

// assign values to list members
void assign_members(list_t *list, node_t *begin_node, node_t *end_node,
					size_t size);

// create and add a node at the beginning of the list
void push_front(list_t *list, const void *data);

// delete the first node of the list
void pop_front(list_t *list, void (*clear_data)(void *data));

// create and add a node at the end of the list
void push_back(list_t *list, const void *data);

// delete the last node of the list
void pop_back(list_t *list, void (*clear_data)(void *data));

// create and insert a node before a given node of the list
void insert(list_t *list, node_t *next_elem, const void *data);

// delete a node from the list
void erase(list_t *list, node_t *elem, void (*clear_data)(void *data));

// clear all data of the list
void clear(list_t *list, void (*clear_data)(void *data));

/* ==========================================================================
   Operations
   ==========================================================================*/

// merge two lists
void merge(list_t *dst_list, list_t *src_list);
