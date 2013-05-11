/* 
 * File:   memwatcher.h
 * Author: florent
 *
 * This is a memory monitoring code. It is very slow (o complexity) for seaching
 * pointer it monitors. But this allows to find potential memory leak directly
 * on some lame Linux environment.
 */

#ifndef MEMWATCHER_H
#define	MEMWATCHER_H

#define MEMWATCHER_LOG

#include "linkedlist.h"

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef MEMWATCHER_LOGALLOC
	/** malloc is overloaded for logging purposes */
#define malloc( s ) __malloc_log( s, __FILE__, __LINE__, __FUNCTION__ )
	/** free is overloaded for logging purposes */
#define free( p ) __free_log( p, __FILE__, __LINE__, __FUNCTION__ )
	/** realloc is overloaded for logging purposes */
#define realloc( p, s ) __realloc_log( p, s, __FILE__, __LINE__, __FUNCTION__ )
#endif
	
#ifdef MEMWATCHER
	/** Memory allocation and tracking */
#define mw_malloc( s ) __mw_malloc( s , __FILE__, __LINE__, __FUNCTION__ )
	/** Memory liberation and tracking */
#define mw_free __mw_free
	/** Memory re-allocation and tracking */
#define mw_realloc( p, s ) __mw_realloc( p, s, __FILE__, __LINE__, __FUNCTION__ )

	/** Memory tracking initialization */
#define mw_init() __mw_init()
	/** Memory tracking end */
#define mw_end() __mw_end()


#define mw_add( p, fi, l, fu ) __mw_add( p, fi, l, fu )
#define mw_rmv( p ) __mw_rmv( p )
#else
#define mw_malloc( s ) malloc( s )
#define mw_free free
#define mw_realloc( p, s ) realloc( p, s )

#define mw_init()
#define mw_end() 

#define mw_add( p, fi, l, fu )
#define mw_rmv( p )
#endif

	/** 
	 * Memwatcher initialization
	 */
	void __mw_init();

	/**
	 * Memwatcher pointer monitoring addition
     * @param ptr Pointer
     * @param file File
     * @param line Line
     * @param function Function
     */
	void __mw_add(void * ptr, const char * file, int line, const char * function);

	/**
	 * Memwatcher pointer allocation + monitoring
     * @param size Size of the pointer
     * @param file File
     * @param line Line
     * @param function Function
     * @return Allocated memory pointer
     */
	void * __mw_malloc(size_t size, const char * file /* = 0 */, int line /* = 0 */, const char * function /* = 0 */);

	/**
	 * Memwatcher pointer monitoring removal
     * @param ptr Pointer to stop monitoring
     */
	void __mw_rmv(void * ptr);

	/**
	 * Memwatcher 
     * @param ptr
     */
	void __mw_free(void * ptr);

	void * __mw_realloc(void * ptr, size_t size, const char * file /* = 0 */, int line /* = 0 */, const char * function /* = 0 */);

	void __mw_end();

	int mw_size();

	void * __malloc_log(size_t s, const char * file /* = 0 */, int line /* = 0 */, const char * function /* = 0 */);

	void * __realloc_log(void * ptr, size_t s, const char * file /* = 0 */, int line /* = 0 */, const char * function /* = 0 */);
	
	void __free_log(void * ptr, const char * file /* = 0 */, int line /* = 0 */, const char * function /* = 0 */);

#ifdef	__cplusplus
}
#endif

#endif	/* MEMWATCHER_H */

