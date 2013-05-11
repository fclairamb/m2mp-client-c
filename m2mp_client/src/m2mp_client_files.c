/* 
 * File:   m2mp_client_files.c
 * Author: florent
 *
 * Created on 26 January 2011, 23:44
 */

#include <stdio.h>
#include <stdlib.h>

#include "m2mp_client_files.h"
#include "m2mp_client_internal.h"
#include "memwatcher.h"

m2mp_client_event ** m2mp_client_files_work(void * ptrClient, void * ptrThis, m2mp_client_event * event) {
	if (!event)
		return NULL;

	//m2mp_client * client = (m2mp_client * ) ptrClient;
	m2mp_client_files * this = (m2mp_client_files *) ptrThis;

	// We never change the event chain
	return NULL;
}

m2mp_client_files * m2mp_client_files_new(m2mp_client * client) {
	m2mp_client_files * this = (m2mp_client_files *) mw_malloc(sizeof (m2mp_client_files));

	m2mp_client_files_init(this, client);


	return this;
}

void m2mp_client_files_init(m2mp_client_files* this, m2mp_client * client) {
	m2mp_client_add_plugin(client, this, m2mp_client_files_work);
}

void m2mp_client_files_delete(m2mp_client_files ** pThis) {
	m2mp_client_files * this = *pThis;

	mw_free(this);

	* pThis = NULL;
}
