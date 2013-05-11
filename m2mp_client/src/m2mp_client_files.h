/* 
 * File:   m2mp_client_files.h
 * Author: florent
 *
 * Created on 26 January 2011, 23:44
 */

#ifndef M2MP_CLIENT_FILES_H
#define	M2MP_CLIENT_FILES_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "logging.h"
#include "linkedlist.h"
#include "m2mp_client.h"

    typedef struct st_m2mp_client_files_file {
        char * file;
        size_t size;
        char * hash;
    } m2mp_client_files_file;

    typedef struct st_m2mp_client_files {
        linkedlist files;
    } m2mp_client_files;

    m2mp_client_files * m2mp_client_files_new(m2mp_client *client);

    void m2mp_client_files_init( m2mp_client_files * this, m2mp_client * client );

    void m2mp_client_files_delete( m2mp_client_files ** this );

    m2mp_client_event ** m2mp_client_files_work(void * client, void * this, m2mp_client_event * event);

#ifdef	__cplusplus
}
#endif

#endif	/* M2MP_CLIENT_FILES_H */

