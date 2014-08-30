/* 
 * File:   main.c
 * Author: Florent
 *
 * Created on 22 janvier 2011, 20:29
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "m2mp_client.h"
#include "m2mp_client_settings.h"
#include "m2mp_client_commands.h"
#include "logging.h"

const char * capabilities_ [] = {"sensor", "echo", "c_client_sample", NULL};

typedef struct st_business_logic {
	struct timeval time;

	struct timeval loadavg_ltime;
	int loadavg_period;

	struct timeval echo_ltime;
	int echo_period;

	int echo_counter;

	unsigned char quit;
} business_logic_t;

void business_logic_init(business_logic_t * this) {
	gettimeofday(& this->time, NULL);
	this->loadavg_ltime = this->time;
	this->loadavg_period = 10;
	this->echo_ltime = this->time;
	this->echo_period = 5;
	this->echo_counter = 0;
	this->quit = 0;
}

void business_logic_code(business_logic_t * this, m2mp_client * client) {
	gettimeofday(& this->time, NULL);

	if (this->time.tv_sec - this->loadavg_ltime.tv_sec > this->loadavg_period) {
		this->loadavg_ltime.tv_sec += this->loadavg_period;
		double loadavg;
		getloadavg(& loadavg, 1);
		char result[256];
		sprintf(result, "%3.2f", loadavg);
		m2mp_client_send_string(client, "sen:loadavg", result);
	}

	if (this->time.tv_sec - this->echo_ltime.tv_sec > this->echo_period) {
		this->echo_ltime.tv_sec += this->echo_period;
		char content[256];
		sprintf(content, "echo_%d", this->echo_counter++);
		m2mp_client_send_string(client, "echo:test", content);
	}
}

business_logic_t buslog;

void signal_handler(int s) {
	printf("Caught signal %d\n", s);
	buslog.quit = 1;
}

/*
 * 
 */
int main(int argc, char** argv) {

	business_logic_init(& buslog);

	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);

	// We create the M2MP client
	m2mp_client * client = m2mp_client_new();
	assert(client);

	// We load the settings management plugin
	m2mp_client_settings * settingsPlugin = m2mp_client_settings_new(client);

	m2mp_client_commands * commandsPlugin = m2mp_client_commands_new(client);
	int commandEventId = m2mp_client_event_commands_get_event_id(commandsPlugin);

	// We load the two plugins into the M2MP client



	// We load or generate or identifier
	char * ident = m2mp_client_settings_get_value(settingsPlugin, "__ident");
	if (!ident) {
		char buffer[1024];
		srand(time(NULL));
		sprintf(buffer, "Linux_%d_%d", (int) time(NULL), rand());
		m2mp_client_settings_set_value(settingsPlugin, "__ident", buffer);
		ident = m2mp_client_settings_get_value(settingsPlugin, "__ident");
		m2mp_client_settings_save_if_necessary(settingsPlugin);
	}

	// We define the ident
	m2mp_client_set_ident(client, ident);

	// We define the capacities of the equipment
	m2mp_client_set_capabilities(client, capabilities_);

	char * client_hostname = "localhost";
	int client_port = 3000;

	LOG(LVL_NOTICE, "M2MP Test client / M2MP Client v%s", m2mp_client_get_version());

	// We init the socket we're going to use...
	LOG(LVL_NOTICE, "Connecting...");
	m2mp_client_connect(client, client_hostname, client_port);



	unsigned char loop = 1;

	unsigned int nbNoEvents = 0;

	while (loop) {

		LOG(LVL_DEBUG, "LOOP / nbNoEvents=%d", nbNoEvents);

		// We load some events from the library (or NULL if there's no event)
		m2mp_client_event * event = m2mp_client_work(client, 4000);

		if (event != NULL) {
			if (event->type == M2MP_CLIENT_EVENT_IDENTIFIED_RESPONSE) {
				m2mp_client_event_identified * identResult = (m2mp_client_event_identified *) event;

				if (identResult->status) {
					LOG(LVL_NOTICE, "We are identified !");
				} else {
					LOG(LVL_NOTICE, "We are not identified !");
				}
			} else if (event->type == M2MP_CLIENT_EVENT_DATA) {
				m2mp_client_event_data * data = (m2mp_client_event_data *) event;

				char * str = m2mp_client_event_data_get_string(data);
				LOG(LVL_NOTICE, "Received on channel \"%s\" : \"%s\".", data->channelName, str);
				free(str);
			} else if (event->type == M2MP_CLIENT_EVENT_DATAARRAY) {
				m2mp_client_event_dataarray * data_array = (m2mp_client_event_dataarray*) event;
				LOG(LVL_NOTICE, "Received on channel \"%s\" : string[].", data_array->channelName);
			} else if (event->type == M2MP_CLIENT_EVENT_CONNECTED) {
				m2mp_client_event_connected * connected = (m2mp_client_event_connected *) event;
				LOG(LVL_NOTICE, "We connected to %s:%d...", connected->serverHostname, connected->serverPort);
			} else if (event->type == M2MP_CLIENT_EVENT_DISCONNECTED) {
				LOG(LVL_NOTICE, "We were disconnected...");
				sleep(5);
				m2mp_client_connect(client, client_hostname, client_port);
			} else if (event->type == M2MP_CLIENT_EVENT_ACK_REQUEST) {
				m2mp_client_event_ack_request * ackRequest = (m2mp_client_event_ack_request *) event;
				LOG(LVL_NOTICE, "We received an ackRequest %d", ackRequest->ackNb);
			} else if (event->type == M2MP_CLIENT_EVENT_ACK_RESPONSE) {
				m2mp_client_event_ack_response * ackResponse = (m2mp_client_event_ack_response *) event;
				LOG(LVL_NOTICE, "We received an ackResponse %d", ackResponse->ackNb);
			} else if (event->type == commandEventId) {
				m2mp_client_event_command * cmd = (m2mp_client_event_command*) event;
				LOG(LVL_NOTICE, "We received a command: %s / %s", cmd->argv[0], cmd->cmdId);
				m2mp_client_event_command_ack(event, commandsPlugin);
			} else {
				LOG(LVL_CRITICAL, "ERROR: Not handled: event->type = %d", event->type);
			}

			m2mp_client_event_delete(client, event);
			nbNoEvents = 0;
		} else {
			business_logic_code(& buslog, client);
			if (buslog.quit) {
				LOG(LVL_CRITICAL, "We are asked to quit");
				break;
			}
		}
	}

	// We unsubscribe the plugins
	m2mp_client_rmv_plugin(client, settingsPlugin);
	m2mp_client_rmv_plugin(client, commandsPlugin);

	// We delete the plugins
	m2mp_client_settings_delete(& settingsPlugin);
	m2mp_client_commands_delete(commandsPlugin);

	// We delete the client
	m2mp_client_delete(& client);
	assert(!client);

	return EXIT_SUCCESS;
}

