/* 
 * File:   memwatcher_logalloc.h
 * Author: florent
 *
 * Created on April 14, 2012, 2:11 AM
 */

#ifndef MEMWATCHER_LOGALLOC_H
#define	MEMWATCHER_LOGALLOC_H

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


#ifdef	__cplusplus
}
#endif

#endif	/* MEMWATCHER_LOGALLOC_H */

