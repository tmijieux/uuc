#include <stdlib.h>

#include "list.hpp"
#include "list_node.hpp"

struct list {
	struct list_node *front_sentinel;
	struct list_node *last;
	
	struct list_node *cursor;
	size_t size;
	unsigned int curpos;
	unsigned int flags;
};


static struct list_node *list_get_node(struct list *list, unsigned int n)
{
	unsigned int k = n;
	struct list_node *node = list->front_sentinel;
	if (list->curpos <= k) {
	k -= list->curpos;
	node = list->cursor;
	}
	for (unsigned int i = 0; i < k; i++)
	node = node_get_next(node);
	list->cursor = node;
	list->curpos = n;
	return node;
}

size_t list_size(struct list *list)
{
	return list->size;
}


struct list *list_new(int flags)
{
	struct list *list = (struct list*) calloc(sizeof(*list), 1);
	list->front_sentinel = node_new(NULL, SENTINEL_NODE);
	node_set_next(list->front_sentinel, node_new(NULL, 1));
	list->flags = flags;
	list->cursor = list->front_sentinel;
	list->curpos = 0;
	list->size = 0;
	return list;
}

void list_free(struct list *list)
{
	struct list_node *node = node_get_next(list->front_sentinel);
	struct list_node *tmp = NULL;
	while (!node_is_sentinel(node)) {
	tmp = node_get_next(node);
	if ((list->flags & LI_FREEMALLOCD) != 0)
		free(node_get_data(node));
	node_free(node);
	node = tmp;
	}
	node_free(node); //the back sentinel
	node_free(list->front_sentinel);
	free(list);
}

void *list_get(struct list *list, unsigned int n)
{
	return node_get_data(list_get_node(list, n));
}

void list_add(struct list *list, void *element)
{
	struct list_node *tmp = node_new(element, 0);
	node_set_next(tmp, node_get_next(list->front_sentinel));
	node_set_next(list->front_sentinel, tmp);
	list->size ++;
}

void list_append(struct list *list, void *element)
{
	int n = list_size(list) + 1;
	list_insert(list, n, element);
}

void list_append_list(struct list *l1, struct list *l2)
{
	for (unsigned int i = 1; i <= list_size(l2); ++i)
	list_append(l1, list_get(l2, i));
}

void list_insert(struct list *list, unsigned int n, void *element)
{
	struct list_node *previous = list_get_node(list, n-1);
	struct list_node *tmp = node_new(element, 0);
	node_set_next(tmp, node_get_next(previous));
	node_set_next(previous, tmp);
	list->size ++;
}

void list_remove(struct list *list, unsigned int n)
{
	struct list_node *previous = list_get_node(list, n-1);
	struct list_node *tmp = node_get_next(previous);
	node_set_next(previous, node_get_next(tmp));
	if ((list->flags & LI_FREEMALLOCD) != 0)
	free(node_get_data(tmp));
	node_free(tmp);
	list->size --;
}


