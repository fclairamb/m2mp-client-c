/* 
 * File:   args.h
 * Author: florent
 *
 * Created on 30 août 2014, 08:08
 */

#ifndef ARGS_H
#define	ARGS_H

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

	typedef struct {
		int max_connected_time;
		bool no_reconnect;
	} args_t;


	int args_parse(args_t * args, int argc, char * argv[]);
	
#ifdef	__cplusplus
}
#endif

#endif	/* ARGS_H */

