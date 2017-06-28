#pragma once

typedef struct ListItem {
	void *data;
	struct ListItem* _next;
} ListItem;

void push(ListItem** head, void* data);
inline void* next_item(ListItem* i) { return i->_next; }