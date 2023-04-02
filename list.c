#include <stdlib.h>
#include <string.h>
#include "list.h"

bool check_list(list_t *list)
{
	if (!list)
		DISPLAY_ERROR(UNINITIALISED_LIST);

	return !list;
}

list_t *init_list(size_t data_size)
{
	list_t *list = malloc(sizeof(list_t));
	CHECK_MEMORY(list, MALLOC_FAILURE);
	*list = (list_t){ NULL, NULL, 0, data_size };
	return list;
}

node_t *init_node(const void *data, const size_t data_size)
{
	node_t *new_node = malloc(sizeof(node_t));
	CHECK_MEMORY(new_node, MALLOC_FAILURE);

	new_node->data = malloc(data_size);
	CHECK_MEMORY(new_node->data, MALLOC_FAILURE);

	memcpy(new_node->data, data, data_size);

	new_node->next = NULL;
	new_node->prev = NULL;

	return new_node;
}

node_t *front(list_t *list)
{
	return list->begin;
}

node_t *back(list_t *list)
{
	return list->end;
}

bool empty(list_t *list)
{
	return list->size == 0;
}

size_t size(list_t *list)
{
	return list->size;
}

void assign_members(list_t *list, node_t *begin_node, node_t *end_node,
					size_t size)
{
	list->begin = begin_node;

	if (list->begin)
		list->begin->prev = NULL;

	list->end = end_node;

	if (list->end)
		list->end->next = NULL;

	list->size = size;
}

void push_front(list_t *list, const void *data)
{
	node_t *new_node = init_node(data, list->data_size);
	new_node->next = list->begin;

	if (list->begin)
		list->begin->prev = new_node;

	list->begin = new_node;
	list->size++;

	if (!list->end)
		list->end = new_node;
}

void pop_front(list_t *list, void (*clear_data)(void *data))
{
	if (!list->begin)
		return;

	node_t *begin = list->begin;
	list->begin = list->begin->next;

	if (list->begin)
		list->begin->prev = NULL;

	clear_data(begin->data);
	free(begin);
	list->size--;
}

void push_back(list_t *list, const void *data)
{
	if (check_list(list))
		return;

	node_t *new_node = init_node(data, list->data_size);
	new_node->prev = list->end;

	if (list->end)
		list->end->next = new_node;

	list->end = new_node;
	list->size++;

	if (!list->begin)
		list->begin = new_node;
}

void pop_back(list_t *list, void (*clear_data)(void *data))
{
	if (!list->end)
		return;

	node_t *end = list->end;
	list->end = list->end->prev;

	if (list->end)
		list->end->next = NULL;

	clear_data(end->data);
	free(end);
	list->size--;
}

void insert(list_t *list, node_t *next_elem, const void *data)
{
	if (!next_elem) {
		push_back(list, data);
		return;
	}

	if (empty(list) || list->begin == next_elem) {
		push_front(list, data);
		return;
	}

	node_t *new_node = init_node(data, list->data_size);

	new_node->next = next_elem;
	new_node->prev = next_elem->prev;
	next_elem->prev->next = new_node;
	next_elem->prev = new_node;

	list->size++;
}

void erase(list_t *list, node_t *elem, void (*clear_data)(void *data))
{
	if (!elem)
		return;

	node_t *prev = elem->prev;
	node_t *next = elem->next;

	if (prev)
		prev->next = next;
	else
		list->begin = next;

	if (next)
		next->prev = prev;
	else
		list->end = prev;

	list->size--;

	clear_data(elem->data);
	free(elem);
}

void clear(list_t *list, void (*clear_data)(void *data))
{
	size_t size = list->size;

	while (size--) {
		node_t *end = list->end;
		list->end = list->end->prev;
		clear_data(end->data);
		free(end);
		list->size--;
	}

	free(list);
}

void merge(list_t *dst_list, list_t *src_list)
{
	dst_list->end->next = src_list->begin;
	src_list->begin->prev = dst_list->end;
	dst_list->end = src_list->end;

	dst_list->size += src_list->size;

	free(src_list);
}
