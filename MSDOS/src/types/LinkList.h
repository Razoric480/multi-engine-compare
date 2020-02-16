#ifndef LINKLIST_H
#define LINKLIST_H

#include "Error.h"

#ifdef LINKLIST_IMPORT
	#define EXTERN
#else
	#define EXTERN extern
#endif

typedef struct LinkList_t *LinkList;

EXTERN ErrorState LinkList_alloc(LinkList* list);
EXTERN void LinkList_free(LinkList* list);

/* Add the element to the furthest right of the list */
EXTERN void LinkList_addLast(LinkList list, void* value);
/* Add the element to the furthest left of the list */
EXTERN void LinkList_addFirst(LinkList list, void* value);
/* Add the element at the specified index in the list, and shifts subsequent elements to the right */
EXTERN void LinkList_addAt(LinkList list, void* value, unsigned int index);

/* Returns the element at the specified index */
EXTERN void* LinkList_getAt(LinkList list, unsigned int index);
/* Returns the first element in the list */
EXTERN void* LinkList_getFirst(LinkList list);
/* Returns the last elements of the list */
EXTERN void* LinkList_getLast(LinkList list);

/* Removes the element at the specified index and returns it. All subsequent elements are shifted to the left. */
EXTERN void* LinkList_removeAt(LinkList list, unsigned int index);
/* Removes the first element of the list and returns it. */
EXTERN void* LinkList_removeFirst(LinkList list);
/* Removes the last elements of the list and returns it. */
EXTERN void* LinkList_removeLast(LinkList list);

/* Sets the element at the specified index, and returns the old value */
EXTERN void* LinkList_setAt(LinkList list, unsigned int index, void* value);
/* Returns the size of the list */
EXTERN unsigned int LinkList_listSize(LinkList list);
/* Clears the list of all elements */
EXTERN void LinkList_clearList(LinkList list);
/* Returns the index of the element if it is found in the list, otherwise -1 */
EXTERN int LinkList_listContains(LinkList list, void* value);

#undef LINKLIST_IMPORT
#undef EXTERN

#endif /* LINKLIST_H */