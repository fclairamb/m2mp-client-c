/* 
 * File:   logging.h
 * Author: florent
 *
 * Created on 23 January 2011, 02:20
 */

#ifndef LOGGING_H
#define	LOGGING_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#define LVL_DEBUG 5
#define LVL_VERBOSE 4
#define LVL_NOTICE 3
#define LVL_WARNING 2
#define LVL_CRITICAL 1
#define LVL_NONE 0

#ifndef LOGGING_LEVEL
#ifdef NDEBUG
#define LOGGING_LEVEL LVL_NOTICE
#else
#define LOGGING_LEVEL LVL_DEBUG
#endif
#endif

#define DO_I_LOG( level ) level <= LOGGING_LEVEL

extern const char * LVL_TO_STRING[];

#define LOG( level, ... ) \
if ( DO_I_LOG( level ) ) { \
    printf( "\n[%20.20s:%4d:%30.30s:%4.4s] " , __FILE__, __LINE__, __FUNCTION__, LVL_TO_STRING[ level ] ); \
	printf( __VA_ARGS__ ); \
    fflush( stdout ); \
}

#define LOG_PRINTF( level, ... ) \
if ( DO_I_LOG( level ) ) \
    printf( __VA_ARGS__ );

#ifdef	__cplusplus
}
#endif

#endif	/* LOGGING_H */

