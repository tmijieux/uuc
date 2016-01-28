#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <assert.h>
// FLAGS for list_new():
#define LI_FREEMALLOCD (1 << 0)  // attempt to free all its elements when
								 // list_free() is called
// END FLAGS

struct list;

// ! !
// NUMBERED FROM 1 !!!!!!!!
// ! !

#define list_addg(typ, li, da)						\
	({ assert(sizeof typ < sizeof (void*)) ;				\
	union { typ t; void *v}e; e.t = da; list_push(li, e.v);})


#define list_getg(typ, li, i)						\
	({ assert(sizeof typ < sizeof (void*)) ;				\
	union { typ t; void *v}e; e.v = list_get((li), (i)); e.t;})

struct list *list_new(int flags);
void list_free(struct list*);
size_t list_size(struct list*); // element count
void * list_get(struct list*, unsigned int i); // returns data
void list_add(struct list*, void*);
void list_insert(struct list*, unsigned int i, void *data);
void list_remove(struct list*, unsigned int i);

void list_append(struct list *list, void *element);
void list_append_list(struct list *l1, struct list *l2);

#endif //LIST_H
