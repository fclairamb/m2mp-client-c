/* 
 * File:   memwatcher.c
 * Author: florent
 *
 * Created on 30 January 2011, 02:00
 */

#include <stdlib.h>
#include <assert.h>

#include "linkedlist.h"
#include "logging.h"


void * __malloc_log( size_t s, const char * file /* = 0 */, int line /* = 0 */, const char * function /* = 0 */ ) {
	void * ptr = malloc(s);
	LOG(LVL_DEBUG, "malloc(%zu):0x%p @%s:%d:%s", s, ptr, file, line, function );
	return ptr;
}

void * __realloc_log( void * p, size_t s, const char * file /* = 0 */, int line /* = 0 */, const char * function /* = 0 */ ) {
	void * ptr = realloc(p, s);
	LOG(LVL_DEBUG,"realloc(0x%p, %zu):0x%p @%s:%d:%s", p, s, ptr, file, line, function );
	return ptr;
}

void __free_log( void * ptr, const char * file /* = 0 */, int line /* = 0 */, const char * function /* = 0 */ ) {
	LOG(LVL_DEBUG, "free(0x%p); @%s:%d:%s", ptr, file, line, function );
	free( ptr );
}

#define MEMWATCHER_CORE

    typedef struct st_mw_meminfo {
        void * ptr;
        const char * file;
        int line;
        const char * function;
    } mw_meminfo;

static linkedlist ptrs_;

void __mw_init() {
	linkedlist_init(& ptrs_);
}

mw_meminfo * mw_search(void * ptr) {
	linkedlist_node * node = linkedlist_get_first_node(& ptrs_);

	while (node) {
		mw_meminfo * info = (mw_meminfo *) node->element;

		if (info->ptr == ptr)
			return info;

		node = node->next;
	}

	return NULL;
}

void __mw_add(void * ptr, const char * file, int line, const char * function) {

	mw_meminfo * info = mw_search(ptr);
	if (info) {
		LOG(LVL_CRITICAL, "The pointer 0x%08X already exists in %s:%d:%s", (unsigned int) (unsigned long) info->ptr, info->file, info->line, info->function);
		assert(!"Pointer added twice !");
	}

	info = malloc(sizeof (mw_meminfo));
	info->ptr = ptr;
	info->file = file;
	info->line = line;
	info->function = function;

	linkedlist_insert_last(& ptrs_, info);
#ifdef MEMWATCHER_LOG
	LOG(LVL_DEBUG, "mw_add( 0x%08X / 0x%08X ); [count=%d] in %s:%d:%s", (unsigned int) (unsigned long) ptr, (unsigned int) (unsigned long) info, (int) linkedlist_get_size(& ptrs_), file, line, function);
#endif
}

void __mw_rmv(void * ptr) {
	mw_meminfo * info = mw_search(ptr);

	if (info) {
#ifdef MEMWATCHER_LOG
		LOG(LVL_DEBUG, "mw_rmv( 0x%08X / 0x%08X ); [count=%d]", (unsigned int) (unsigned long) ptr, (unsigned int) (unsigned long) info, (int) (long) linkedlist_get_size(& ptrs_));
#endif
		linkedlist_remove(& ptrs_, info);
		free(info);
	} else {
		LOG(LVL_WARNING, "WARNING: mw_rmv( 0x%08X ); - Pointer wasn't added", (unsigned int) (unsigned long) ptr);
		assert(!"Pointer not added !");
	}
}


void * __mw_malloc(size_t size, const char * file /* = 0 */, int line /* = 0 */, const char * function /* = 0 */) {
	void * ptr = malloc(size);

	__mw_add(ptr, file, line, function);

	return ptr;
}

void * __mw_realloc(void * ptr, size_t size, const char * file /* = 0 */, int line /* = 0 */, const char * function /* = 0 */) {

	__mw_rmv(ptr);

	ptr = realloc(ptr, size);

	__mw_add(ptr, file, line, function);

	return ptr;
}


void __mw_free(void * ptr) {

	__mw_rmv(ptr);

	free(ptr);
}

void mw_meminfo_delete_void( void * ptr ) {
	free( ptr );
}

void __mw_end() {
	linkedlist_node * node = linkedlist_get_first_node(& ptrs_);

	while (node) {
		mw_meminfo * info = (mw_meminfo *) node->element;

		LOG(LVL_CRITICAL, "ERROR: Unreleased pointer 0x%08X created in %s:%d:%s", (unsigned int) (unsigned long) info->ptr, info->file, info->line, info->function);

		node = node->next;
	}

	linkedlist_empty(& ptrs_, mw_meminfo_delete_void);
}

int mw_size() {
#ifdef MEMWATCHER
	return linkedlist_get_size(& ptrs_);
#else
	return 0;
#endif
}
