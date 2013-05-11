/* 
 * File:   linked_list.h
 * Author: florent
 *
 * Created on 24 January 2011, 21:56
 */

#ifndef LINKED_LIST_H
#define	LINKED_LIST_H

#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct st_linkedlist_node {
        struct st_linkedlist_node * next;
        void * element;
    } linkedlist_node;

    typedef struct st_linkedlist {
        linkedlist_node * first;
        linkedlist_node * last;
        size_t size;
    } linkedlist;

    linkedlist * linkedlist_new();

    linkedlist_node * linkedlist_node_new();

    linkedlist_node * linkedlist_get_first_node(linkedlist * this);

    void linkedlist_node_delete(linkedlist_node * this);

    void linkedlist_init(linkedlist * this);

    void linkedlist_empty(linkedlist * this, void (*func)(void * deleter));

    void linkedlist_delete(linkedlist * * this);

    void linkedlist_insert_first(linkedlist * this, void * element);

    void linkedlist_insert_last(linkedlist * this, void * element);

    void * linkedlist_pop_first(linkedlist * this);

    //    void * linkedlist_pop_last(linkedlist * this);

    void ** linkedlist_get_array(linkedlist * this);

    size_t linkedlist_get_size(linkedlist * this);

    unsigned char linkedlist_remove( linkedlist * this, void * element );

#ifdef	__cplusplus
}
#endif

#endif	/* LINKED_LIST_H */

