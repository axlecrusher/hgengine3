#include <stdlib.h>
#include <LinkedList.h>

void push(ListItem** head, void* data) {
	ListItem *item = malloc(sizeof(*item));
	item->data = data;
	item->_next = *head;
	*head = item;
}