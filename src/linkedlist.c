/* 
 * File:   linked_list.c
 * Author: florent
 *
 * Created on 24 January 2011, 21:56
 */

#include "linkedlist.h"

// Linkedlist cannot use memwatcher because the memwatcher relies on the linkedlist
// to store all the pointers.

linkedlist * linkedlist_new() {
    linkedlist * this = (linkedlist *) malloc(sizeof (linkedlist));

    linkedlist_init(this);

    return this;
}

linkedlist_node * linkedlist_node_new() {
	// linked_list doesn't use memwatcher as it is used in the memwatcher
    return (linkedlist_node *) malloc(sizeof (linkedlist_node));
}

void linkedlist_node_delete(linkedlist_node* this) {
	// linked_list doesn't use memwatcher as it is used in the memwatcher
    free(this);
}

void linkedlist_init(linkedlist * this) {
    this->first = NULL;
    this->last = NULL;
    this->size = 0;
}

void linkedlist_empty(linkedlist * this, void (*deleter)(void * elt)) {
    void * elt = NULL;
    while ((elt = linkedlist_pop_first(this)) != NULL) {
        if (deleter != NULL)
            deleter(elt);
    }
}

void linkedlist_delete(linkedlist** this) {
    linkedlist_empty(*this, NULL);
    free(* this);
    *this = NULL;
}

void linkedlist_insert_first(linkedlist* this, void* elt) {

    linkedlist_node * first = this->first;

    linkedlist_node * node = linkedlist_node_new();
    node->element = elt;
    node->next = first;

    this->first = node;

    this->size++;
}

void linkedlist_insert_last(linkedlist* this, void* element) {

    linkedlist_node * last = this->last;

    linkedlist_node * node = linkedlist_node_new();
    node->element = element;
    node->next = NULL;

    this->last = node;
    if (!this->first)
        this->first = node;

    if (last != NULL)
        last->next = node;

    this->size++;
}

void * linkedlist_pop_first(linkedlist* this) {
    linkedlist_node * first = this->first;

    if (!first)
        return NULL;

    this->first = first->next;

    void * elt = first->element;

    linkedlist_node_delete(first);

    this->size--;

    if (this->size == 0)
        this->last = NULL;

    return elt;
}

/*
void * linkedlist_pop_last(linkedlist* this) {

}
 */

void ** linkedlist_get_array(linkedlist* this) {
    // This doesn't have to be the real malloc as it's not necessary for the linkedlist management
    void ** nodes = (void **) malloc(sizeof (void*) * (this->size + 1));

    int size = this->size;

    int i;

    linkedlist_node * node = this->first;

    for (i = 0; i < size; ++i) {
        nodes[i] = node->element;
        node = node->next;
    }

    nodes[ size ] = NULL;

    return nodes;
}

size_t linkedlist_get_size(linkedlist* this) {
    return this->size;
}

linkedlist_node * linkedlist_get_first_node(linkedlist * this) {
    return this->first;
}

unsigned char linkedlist_remove(linkedlist * this, void * element) {
    linkedlist_node * node = this->first;

    linkedlist_node * lastNode = NULL;
    while (node) {
        if (node->element == element) {

            if (this->first == node)
                this->first = node->next;

            if (this->last == node)
                this->last = lastNode;

            if (lastNode != NULL)
                lastNode->next = node->next;

            linkedlist_node_delete(node);
            this->size--;
            return 1;
        }

        lastNode = node;
        node = node->next;
    }

    return 0;
}

