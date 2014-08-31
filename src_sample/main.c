/* 
 * File:   main.c
 * Author: Florent
 *
 * Created on 22 janvier 2011, 20:29
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <malloc.h>
#include "m2mp_client.h"
#include "m2mp_client_settings.h"
#include "m2mp_client_commands.h"
#include "logging.h"
#include "args.h"

const char * capabilities_ [] = {"sensor", "echo", "c_client_sample", NULL};

typedef struct {
	time_t loadavg_time;
	int loadavg_period;

	time_t echo_time;
	int echo_period;
	int echo_counter;

	time_t uptime_time;
	int uptime_period;

	time_t connected_time;
	time_t identified_time;

	bool quit;

	args_t args;

	char **servers;
	int server_index;
	int server_sleep_between_connections;
} business_logic_t;

void business_logic_init(business_logic_t * this) {
	time_t now = time(NULL);


	this->loadavg_period = 60;
	this->loadavg_time = now - this->loadavg_period;

	this->echo_period = 10;
	this->echo_time = now - this->echo_period;
	this->echo_counter = 0;

	this->uptime_period = 300;
	this->uptime_time = now - this->uptime_period;

	this->quit = false;
	this->identified_time = 0;

	this->servers = NULL;
	this->server_index = 0;
}

void business_logic_close(business_logic_t * this) {
	if (this->servers) {

		char ** ptr;

		for (ptr = this->servers; *ptr; ptr++) {
			free(*ptr);
		}


		free(this->servers);
	}
}

int business_logic_connect(business_logic_t * this, m2mp_client * client) {
	char buffer[BUFSIZ];
	char * server = this->servers[this->server_index++];

	if (!server) {
		this->server_index = 0;
		server = this->servers[this->server_index++];
	}

	strcpy(buffer, server);

	char * host = strtok(buffer, ":");
	int port = atoi(strtok(NULL, ":"));

	if (this->server_sleep_between_connections) {
		LOG(LVL_NOTICE, "Sleeping for %d seconds...", this->server_sleep_between_connections);
		sleep(this->server_sleep_between_connections);
	}

	return m2mp_client_connect(client, host, port);
}

void free_time_to_think(business_logic_t * this, m2mp_client * client) {
	time_t now = time(NULL);
	if (!this->identified_time) {

		// We wait at most for 30s for an identification response
		if (this->connected_time && (now - this->connected_time) > 5) {
			LOG(LVL_WARNING, "Identification request timeout, disconnecting...");
			this->connected_time = 0;
			m2mp_client_disconnect(client);
		}

		// We don't do any kind of reporting until we're connected
		return;
	}

	if (now - this->loadavg_time >= this->loadavg_period) {
		this->loadavg_time += this->loadavg_period;
		double loadavg;
		getloadavg(& loadavg, 1);
		char value[256];
		sprintf(value, "%3.2f", loadavg);
		m2mp_client_send_string(client, "sen:loadavg", value);
	}

	if (now - this->echo_time >= this->echo_period) {
		this->echo_time += this->echo_period;
		char content[256];
		sprintf(content, "echo_%d", this->echo_counter++);
		m2mp_client_send_string(client, "echo:test", content);
	}

	if (now - this->uptime_time >= this->uptime_period) {
		this->uptime_time += this->uptime_period;
		char value[256];
		struct sysinfo info;
		sysinfo(& info);
		sprintf(value, "%ld", info.uptime);
		m2mp_client_send_string(client, "sen:uptime", value);
	}

	if (this->identified_time && this->args.max_connected_time && now - this->identified_time > this->args.max_connected_time) {
		LOG(LVL_NOTICE, "We've been connected for too long! Requesting disconnection...");
		m2mp_client_send_string(client, "_special", "disconnect_me");
		this->identified_time = 0;
	}
}

business_logic_t buslog;

void signal_handler(int s) {
	printf("Caught signal %s (%d)\n", strsignal(s), s);
	buslog.quit = 1;
}

/*
 * 
 */
int main(int argc, char** argv) {

	business_logic_init(& buslog);

	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);


	// We parse the parameters

	args_parse(& buslog.args, argc, argv);

	// We create the M2MP client
	m2mp_client * client = m2mp_client_new();



	// We load the settings management plugin
	m2mp_client_settings * settingsPlugin = m2mp_client_settings_new(client);

	m2mp_client_commands * commandsPlugin = m2mp_client_commands_new(client);
	int commandEventId = m2mp_client_event_commands_get_event_id(commandsPlugin);

	// We load the two plugins into the M2MP client



	{// We load or generate or identifier
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
	}

	{ // We might have to change the list of servers
		if (buslog.args.servers) {
			m2mp_client_settings_set_value(settingsPlugin, "servers", buslog.args.servers);
			m2mp_client_settings_save_if_necessary(settingsPlugin);
		}
	}

	{ // We load or create the list of servers
		char * servers_values = m2mp_client_settings_get_value(settingsPlugin, "servers");
		if (!servers_values) {
			servers_values = "localhost:3010,localhost:3000";
			m2mp_client_settings_set_value(settingsPlugin, "servers", servers_values);
			m2mp_client_settings_save_if_necessary(settingsPlugin);
		}
		char * alloc = strdup(servers_values);
		char * tok;
		int i = 0;
		buslog.servers = malloc(sizeof (char *));
		while ((tok = strtok(i ? NULL : alloc, ","))) {
			buslog.servers = (char **) realloc(buslog.servers, sizeof (char *) * (i + 1));
			buslog.servers[i++] = strdup(tok);
		}
		buslog.servers[i] = NULL;
		free(alloc);
	}

	// We define the capabilities of the equipment
	m2mp_client_set_capabilities(client, capabilities_);

	LOG(LVL_NOTICE, "M2MP Test client / M2MP Client v%s", m2mp_client_get_version());

	// We init the socket we're going to use...
	LOG(LVL_NOTICE, "Connecting...");
	business_logic_connect(&buslog, client);

	while (!buslog.quit) {

		// We load some events from the library (or NULL if there's no event)
		m2mp_client_event * event = m2mp_client_work(client, 4000);

		if (event != NULL) {
			if (event->type == M2MP_CLIENT_EVENT_IDENTIFIED_RESPONSE) {
				m2mp_client_event_identified * identResult = (m2mp_client_event_identified *) event;

				if (identResult->status) {
					LOG(LVL_NOTICE, "We are identified !");
					buslog.identified_time = time(NULL);
				} else {
					LOG(LVL_NOTICE, "We are not identified !");
					m2mp_client_disconnect(client);
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
				buslog.connected_time = time(NULL);
				buslog.server_sleep_between_connections = 0;
			} else if (event->type == M2MP_CLIENT_EVENT_DISCONNECTED) {
				LOG(LVL_NOTICE, "We were disconnected...");
				if (!buslog.quit) { // If we are not in quitting process
					if (buslog.args.no_reconnect) { // And if we are not forbidden to reconnect
						LOG(LVL_NOTICE, "We have to quit ! (--no-reconnect option)");
						buslog.quit = true;
					} else { // We reconnect
						LOG(LVL_NOTICE, "Reconnecting...");
						business_logic_connect(&buslog, client);
					}
				}
				buslog.connected_time = 0;
				buslog.identified_time = 0;
				if (buslog.server_sleep_between_connections < 60) {
					buslog.server_sleep_between_connections += 1;
				}
			} else if (event->type == M2MP_CLIENT_EVENT_ACK_REQUEST) {
				m2mp_client_event_ack_request * ackRequest = (m2mp_client_event_ack_request *) event;
				LOG(LVL_NOTICE, "We received an ackRequest %d", ackRequest->ackNb);
			} else if (event->type == M2MP_CLIENT_EVENT_ACK_RESPONSE) {
				m2mp_client_event_ack_response * ackResponse = (m2mp_client_event_ack_response *) event;
				LOG(LVL_NOTICE, "We received an ackResponse %d", ackResponse->ackNb);
			} else if (event->type == M2MP_CLIENT_EVENT_POLL_ERROR) {
				LOG(LVL_WARNING, "We got an internal poll error");
			} else if (event->type == commandEventId) {
				m2mp_client_event_command * cmd = (m2mp_client_event_command*) event;
				LOG(LVL_NOTICE, "We received a command: %s / %s", cmd->argv[0], cmd->cmdId);
				if (!strcmp(cmd->argv[0], "shell")) {
					LOG(LVL_NOTICE, "Executing system(\"%s\");", cmd->argv[1]);
					int err;
					if ((err = system(cmd->argv[1]))) {
						char buffer[BUFSIZ];
						sprintf(buffer, "Error with system: %s", strerror(err));
						m2mp_client_send_string(client, "sen:log", buffer);
					}
				} else if (!strcmp(cmd->argv[0], "restart")) {
					int err;
					if ((err = system("/sbin/reboot"))) {
						char buffer[BUFSIZ];
						sprintf(buffer, "Error with system: %s", strerror(err));
						m2mp_client_send_string(client, "sen:log", buffer);
					};
				}
				m2mp_client_event_command_ack(event, commandsPlugin);
			} else {
				LOG(LVL_CRITICAL, "ERROR: Not handled: event->type = %d", event->type);
			}

			m2mp_client_event_delete(client, event);
		} else {
			free_time_to_think(& buslog, client);
		}
	}

	// We unsubscribe the plugins
	m2mp_client_rmv_plugin(client, settingsPlugin);
	m2mp_client_rmv_plugin(client, commandsPlugin);

	// We delete the plugins
	m2mp_client_settings_delete(settingsPlugin);
	m2mp_client_commands_delete(commandsPlugin);

	// We delete the client
	m2mp_client_delete(& client);

	// We delete the business logic allocated data
	business_logic_close(& buslog);

	return EXIT_SUCCESS;
}

