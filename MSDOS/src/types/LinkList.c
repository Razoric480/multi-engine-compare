#include <stdio.h>
#include <alloc.h>

#define LINKLIST_IMPORT
#include "types\LinkList.h"

typedef struct Node Node;

struct Node {
	Node* next;
	Node* prev;
	void* value;
};

typedef struct LinkList_t {
	Node* first;
	Node* last;
	unsigned int len;
} LinkList_t;

static Node* getNodeAt(LinkList list, unsigned int index) {
    unsigned int count;
    Node *node;

    if(list == 0 || list->len == 0) {
        return 0;
    }

    if(index > list->len-1) {
        return 0;
    }

    count = 0;
    node = list->first;

    if(node == 0) {
        return 0;
    }

    while(node->next != 0 && count < index) {
        node = node->next;
        count++;
    }

    return node;
}

ErrorState LinkList_alloc(LinkList* list) {
    (*list)=(LinkList)calloc(1, sizeof(LinkList_t));
    if(!(*list)) {
        return ERR_OUT_OF_MEMORY;
    }

    return ERR_SUCCESS;
}

void LinkList_clearList(LinkList list) {
    Node *current, *freed;

    if(list == 0 || list->len == 0) {
        return;
    }

    current = list->first;
    do {
        if(current != 0) {
            freed = current;
            current = current->next;
            free(freed);
            list->len--;
        }
    } while(current != 0);
}

void LinkList_free(LinkList* list) {
    if((*list)) {
        LinkList_clearList(*list);
        free(*list);
        *list = 0;
    }
}

void LinkList_addLast(LinkList list, void* value) {
    Node *newNode;

    if(list == 0) {
        return;
    }

    newNode = (Node*)calloc(1, sizeof(Node));
	if (newNode == 0) {
		return;
	}
    newNode->value = value;

    if(list->len > 0) {
        newNode->prev = list->last;
        list->last->next = newNode;
        list->last = newNode;
    }
    else {
        list->first = newNode;
        list->last = newNode;
    }

    list->len++;
}

void LinkList_addFirst(LinkList list, void* value) {
    Node *newNode;

    if(list == 0) {
        return;
    }

    newNode = (Node*)calloc(1, sizeof(Node));
	if (newNode == 0) {
		return;
	}
    newNode->value = value;

    if(list->first != 0) {
        newNode->next = list->first;
        list->first->prev=newNode;
        list->first=newNode;
    }
    else {
        list->first = newNode;
        list->last = newNode;
    }
    
    list->len++;
}

void LinkList_addAt(LinkList list, void* value, unsigned int index) {
    Node *newNode, *nodeAt;

    if(list == 0) {
        return;
    }

    if(index > list->len-1) {
        return;
    }

    newNode = (Node*)calloc(1, sizeof(Node));
	if (newNode == 0) {
		return;
	}
    newNode->value = value;

    if(index == 0) {
        nodeAt = list->first;
    }
    else if(index == list->len-1) {
        nodeAt = list->last;
    }
    else {
        nodeAt = getNodeAt(list, index);
    }

    newNode->prev = nodeAt->prev;
    newNode->next = nodeAt->next;
    nodeAt->prev->next = newNode;
    nodeAt->prev = newNode;

    if(index == 0) {
        list->first = newNode;
    }

    list->len++;
}

void* LinkList_getAt(LinkList list, unsigned int index) {
    if(list == 0 || list->len == 0) {
        return 0;
    }

    if(index > list->len-1) {
        return 0;
    }

    if(index == 0) {
        return list->first->value;
    }

    if(index == list->len-1) {
        return list->last->value;
    }

    return getNodeAt(list, index)->value;
}

void* LinkList_getFirst(LinkList list) {
    if(list == 0 || list->len == 0) {
        return 0;
    }

    return list->first->value;
}

void* LinkList_getLast(LinkList list) {
    if(list == 0 || list->len == 0) {
        return 0;
    }

    return list->last->value;
}

void* LinkList_removeAt(LinkList list, unsigned int index) {
    Node *current;
    void* value;

    if(list == 0 || list->len == 0) {
        return 0;
    }

    if(index > list->len-1) {
        return 0;
    }

    if(index == 0) {
        return LinkList_removeFirst(list);
    }

    if(index == list->len-1) {
        return LinkList_removeLast(list);
    }

    current = getNodeAt(list, index);
    current->prev->next = current->next;
    current->next->prev = current->prev;

    value = current->value;
    free(current);

    list->len--;

    return value;
}

void* LinkList_removeFirst(LinkList list) {
    Node *current;
    void* value;

    if(list == 0 || list->len == 0) {
        return 0;
    }

    current = list->first;
    if(current->next != 0) {
        list->first = list->first->next;
        list->first->prev = 0;
    }
    else {
        list->first = 0;
        list->last = 0;
    }
        
    value = current->value;
    free(current);

    list->len--;

    return value;
}

void* LinkList_removeLast(LinkList list) {
    Node *current;
    void* value;

    if(list == 0 || list->len == 0) {
        return 0;
    }

    current = list->last;
    if(current->prev != 0) {
        list->last = list->last->prev;
        list->last->next = 0;
    }
    else {
        list->last = 0;
        list->first = 0;
    }

    value = current->value;
    free(current);

    list->len--;

    return value;
}

void* LinkList_setAt(LinkList list, unsigned int index, void* value) {
    Node *current;
    void *oldValue;

    if(list == 0 || list->len == 0 || index > list->len-1) {
        return 0;
    }

    if(index == 0) {
        oldValue = list->first->value;
        list->first->value = value;
        return oldValue;
    }

    if(index == list->len-1) {
        oldValue = list->last->value;
        list->last->value = value;
        return oldValue;
    }

    current = getNodeAt(list, index);
    oldValue = current->value;
    current->value = value;
    return oldValue;
}

unsigned int LinkList_listSize(LinkList list) {
    if(!list) {
        return 0;
    }
    
    return list->len;
}

/* Returns the index of the element if it is found in the list, otherwise -1 */
int LinkList_listContains(LinkList list, void* value) {
    Node *current;
    int count = 0;

    if(list == 0 || list->len == 0) {
        return -1;
    }

    current = list->first;
    do {
        if(current->value == value) {
            return count;
        }

        count++;
        current = current->next;
    } while(current->next != 0);

    return -1;
}