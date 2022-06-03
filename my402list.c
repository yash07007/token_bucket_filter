/*
 * Author:      Yash Solanki (ysolanki@usc.edu)
 *
 * @(#)$Id: my402list.c,v 1.0 2 $
 */

#include <stdlib.h>
#include "my402list.h"

int  My402ListLength(My402List* list) {
	if(list == NULL) return 0;
	return list->num_members;
}

int  My402ListEmpty(My402List* list) {
	return My402ListLength(list) == 0;
}

int  My402ListAppend(My402List* list, void* obj) {
	My402ListElem* newElem = (My402ListElem*)malloc(sizeof(My402ListElem));
        if(!newElem) return 0;
        newElem->obj = obj;
	if(My402ListEmpty(list)) {
		My402ListInit(list);
		list->anchor.next = newElem;
		newElem->prev = &list->anchor;
		list->anchor.prev = newElem;
		newElem->next = &list->anchor;
		list->num_members++;
		return 1;
	}
	My402ListElem* lastElem = My402ListLast(list);
	lastElem->next = newElem;
	newElem->next = &list->anchor;
	list->anchor.prev = newElem;
	newElem->prev = lastElem;
	list->num_members++;
	return 1;
}

int  My402ListPrepend(My402List* list, void* obj) {
	My402ListElem* newElem = (My402ListElem*)malloc(sizeof(My402ListElem));
        if(!newElem) return 0;
        newElem->obj = obj;
        if(My402ListEmpty(list)) {
                My402ListInit(list);
                list->anchor.prev = newElem;
                newElem->prev = &list->anchor;
                list->anchor.next = newElem;
                newElem->next = &list->anchor;
                list->num_members++;
                return 1;
        }
        My402ListElem* firstElem = My402ListFirst(list);
        firstElem->prev = newElem;
        newElem->prev = &list->anchor;
        list->anchor.next = newElem;
        newElem->next = firstElem;
        list->num_members++;
        return 1;
}

void My402ListUnlink(My402List* list, My402ListElem* elem) {
	if(My402ListEmpty(list) || elem == NULL) return;
	My402ListElem* prev = elem->prev;
	My402ListElem* next = elem->next;
	if(prev) prev->next = next;
	if(next) next->prev = prev;
	list->num_members--;
	free(elem);
}

void My402ListUnlinkAll(My402List* list) {
	if(My402ListEmpty(list)) return;
        while(My402ListFirst(list)) My402ListUnlink(list, My402ListFirst(list));
	list->anchor.next = &list->anchor;
        list->anchor.prev = &list->anchor;
}

int  My402ListInsertAfter(My402List* list, void* obj, My402ListElem* elem) {
	if(elem == NULL) {
                return My402ListAppend(list, obj);
        }
        My402ListElem* newElem = (My402ListElem*)malloc(sizeof(My402ListElem));
        if(!newElem) return 0;
        newElem->obj = obj;
        My402ListElem* next = elem->next;
        elem->next = newElem;
        newElem->next = next;
        next->prev = newElem;
        newElem->prev = elem;
        list->num_members++;
        return 1;
}

int  My402ListInsertBefore(My402List* list, void* obj, My402ListElem* elem) {
	if(elem == NULL) {
		return My402ListPrepend(list, obj);
	}
	My402ListElem* newElem = (My402ListElem*)malloc(sizeof(My402ListElem));
	if(!newElem) return 0;
	newElem->obj = obj;
	My402ListElem* prev = elem->prev;
	prev->next = newElem;
	newElem->next = elem;
	elem->prev = newElem;
	newElem->prev = prev;
	list->num_members++;
        return 1;
}

My402ListElem *My402ListFirst(My402List* list) {
	if(My402ListEmpty(list)) return NULL;
	return list->anchor.next;
}

My402ListElem *My402ListLast(My402List* list) {
	if(My402ListEmpty(list)) return NULL;
	return list->anchor.prev;
}

My402ListElem *My402ListNext(My402List* list, My402ListElem* elem) {
	if(My402ListLast(list) == elem) return NULL;
	return elem->next;
}

My402ListElem *My402ListPrev(My402List* list, My402ListElem* elem) {
	if(My402ListFirst(list) == elem) return NULL;
	return elem->prev;
}

My402ListElem *My402ListFind(My402List* list, void* obj) {
	My402ListElem *currElem = NULL;
	for(currElem=My402ListFirst(list); currElem != NULL; currElem = My402ListNext(list, currElem)) {
		if(currElem->obj == obj) {
			return currElem;
		}
	}
	return NULL;
}

int My402ListInit(My402List* list) {
	list->num_members = 0;
	list->anchor.obj = NULL;
	list->anchor.next = &list->anchor;
	list->anchor.prev = &list->anchor;
	return 1;
}

