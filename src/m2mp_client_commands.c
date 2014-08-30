#include "m2mp_client_commands.h"
#include "linkedlist.h"

m2mp_client_event_command * m2mp_client_event_command_new(int typeId) {
	m2mp_client_event_command * cmd= (m2mp_client_event_command *) malloc(sizeof ( m2mp_client_event_command));
	cmd->base.type = typeId;
	return cmd;
}

void m2mp_client_commands_delete(m2mp_client_commands* this) {
	free(this);
}

m2mp_client_commands * m2mp_client_commands_new(m2mp_client * client) {
	m2mp_client_commands * this = (m2mp_client_commands*) malloc(sizeof (m2mp_client_commands));
	m2mp_client_commands_init(this, client);
	return this;
}

void m2mp_client_commands_init(m2mp_client_commands * this, m2mp_client * client) {
	this->client = client;
	this->command_event_id = m2mp_client_register_event_type(this->client);
	m2mp_client_add_plugin(client, this, m2mp_client_commands_work);
	m2mp_client_register_event_destructor(this->client, this->command_event_id, m2mp_client_event_command_delete);
}

m2mp_client_event ** m2mp_client_commands_work(void* ptrClient, void* ptrThis, m2mp_client_event* event) {
	if (!event)
		return NULL;

	m2mp_client_commands * this = (m2mp_client_commands *) ptrThis;
	m2mp_client_event ** events = NULL;
	if (event->type == M2MP_CLIENT_EVENT_DATAARRAY) {
		m2mp_client_event_dataarray * eventDataArray = (m2mp_client_event_dataarray*) event;
		if (!strcmp(eventDataArray->channelName, "_cmd")) {
			int size = eventDataArray->size;
			if (size > 2) {
				char * type = m2mp_client_event_dataarray_get_string(eventDataArray, 0);
				if (!strcmp(type, "e")) {
					events = malloc(sizeof ( m2mp_client_event *) * 3);
					// We keep the previous event
					events[0] = event;

					m2mp_client_event_command * eventCmd = m2mp_client_event_command_new(this->command_event_id);
					
					// We create a new event
					events[1] = (m2mp_client_event*) eventCmd;

					// ANd add the null pointer
					events[2] = NULL;

					eventCmd->cmdId = m2mp_client_event_dataarray_get_string(eventDataArray, 1);
					eventCmd->argc = eventDataArray->size - 2;
					eventCmd->argv = (char **) malloc(sizeof (char*) * (eventCmd->argc + 1));
					eventCmd->argv[eventCmd->argc ] = NULL;
					{
						int i = 0;
						for (i = 0; i < eventCmd->argc; i++) {
							eventCmd->argv[i] = m2mp_client_event_dataarray_get_string(eventDataArray, i + 2);
						}
					}
				}
				free(type);
			}
		}
	}

	return events;
}

void m2mp_client_event_command_delete(m2mp_client_event* e) {
	m2mp_client_event_command * event = (m2mp_client_event_command*) e;
	free(event->cmdId);
	int i;
	for (i = 0; i < event->argc; i++) {
		free(event->argv[i]);
	}
	free(event->argv);
	free(event);
}

int m2mp_client_event_commands_get_event_id(m2mp_client_commands * cmds) {
	return cmds->command_event_id;
}


void m2mp_client_event_command_ack( m2mp_client_event *event, m2mp_client_commands *plugin ) {
	linkedlist * list = linkedlist_new();
	
	m2mp_client_event_command * cmdEvent = (m2mp_client_event_command*) event;
	
	linkedlist_insert_last( list, "a");
	linkedlist_insert_last( list, cmdEvent->cmdId );
	
	const char ** argv = (const char **) linkedlist_get_array( list );
	m2mp_client_send_string_array( plugin->client, "_cmd", argv );
	free( argv );
	linkedlist_delete(& list );
}
