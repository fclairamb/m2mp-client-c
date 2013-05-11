/* 
 * File:   m2mp_client_commands.h
 * Author: florent
 *
 * Created on April 23, 2012, 12:01 AM
 */

#ifndef M2MP_CLIENT_COMMANDS_H
#define	M2MP_CLIENT_COMMANDS_H

#include "m2mp_client.h"


#ifdef	__cplusplus
extern "C" {
#endif

	typedef struct st_m2mp_client_event_command {
		m2mp_client_event base;
		char * cmdId;
		int argc;
		char ** argv;
	} m2mp_client_event_command;

	typedef struct st_m2mp_client_commands {
		m2mp_client * client;
		int command_event_id;
	} m2mp_client_commands;

	m2mp_client_commands * m2mp_client_commands_new(m2mp_client * client);
	void m2mp_client_commands_init(m2mp_client_commands * this, m2mp_client * client);
	m2mp_client_event ** m2mp_client_commands_work(void * ptrClient, void * ptrThis, m2mp_client_event * event);
	
	int m2mp_client_event_commands_get_event_id(m2mp_client_commands * this);
	void m2mp_client_commands_delete(m2mp_client_commands * this);

	

	m2mp_client_event_command * m2mp_client_event_command_new( int typeId);
	void m2mp_client_event_command_ack( m2mp_client_event *event, m2mp_client_commands *plugin );
	void m2mp_client_event_command_delete(m2mp_client_event * event);
#ifdef	__cplusplus
}
#endif

#endif	/* M2MP_CLIENT_COMMANDS_H */

