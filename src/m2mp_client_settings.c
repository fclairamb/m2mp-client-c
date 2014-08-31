/* 
 * File:   m2mp_client_settings.c
 * Author: florent
 *
 * Created on 27 January 2011, 23:09
 */

#include <stdio.h>
#include <stdlib.h>

#include "m2mp_client_settings.h"
#include "m2mp_client_internal.h"
#include "str.h"

m2mp_client_settings * m2mp_client_settings_new(m2mp_client * client) {
	m2mp_client_settings * this = (m2mp_client_settings *) malloc(sizeof (m2mp_client_settings));

	m2mp_client_settings_init(this, client);

	m2mp_client_settings_load(this);

	return this;
}

void m2mp_client_settings_init(m2mp_client_settings * this, m2mp_client * client) {
	this->modified = 0;
	this->client = client;
	dictionnary_init(& this->settings);

	m2mp_client_add_plugin(client, this, m2mp_client_settings_work);
}

void m2mp_client_settings_delete(m2mp_client_settings * this) {
	dictionnary_clear(& this->settings);
	free(this);
}

void m2mp_client_settings_treat_data(m2mp_client_settings * this, m2mp_client_event_data *dataEvent) {
	char * str = m2mp_client_event_data_get_string(dataEvent);
	LOG(LVL_DEBUG, "data = \"%s\"", str);
	free(str);
}

void m2mp_client_settings_treat_data_array(m2mp_client_settings * this, m2mp_client_event_dataarray *dataArrayEvent) {
	int size = dataArrayEvent->size;

	BOOL get = FALSE;
	BOOL set = FALSE;
	BOOL getAll = FALSE;

	if (size > 0) {
		char * sFirstCell = m2mp_client_event_dataarray_get_string(dataArrayEvent, 0);
		LOG(LVL_DEBUG, "cell[0] = \"%s\"", sFirstCell);
		if (!strcmp(sFirstCell, "g"))
			get = TRUE;
		else if (!strcmp(sFirstCell, "s"))
			set = TRUE;
		else if (!strcmp(sFirstCell, "sg")) {
			get = TRUE;
			set = TRUE;
		} else if (!strcmp(sFirstCell, "ga")) {
			get = TRUE;
			getAll = TRUE;
		}

		free(sFirstCell);
	}

	linkedlist * getCellsList = NULL;

	if (get) {
		getCellsList = linkedlist_new();
		//mw_add(getCellsList, __FILE__, __LINE__, __FUNCTION__);
		linkedlist_insert_last(getCellsList, strdup("g"));
	}

	int i;
	for (i = 1; i < size; ++i) {
		char * sCell = m2mp_client_event_dataarray_get_string(dataArrayEvent, i);
		int j;

		char * name = NULL, * value = NULL;

		LOG(LVL_DEBUG, "cell[%d] = \"%s\"", i, sCell);

		for (j = 0; sCell[j]; j++) {
			if (sCell[j] == '=') {
				sCell[j] = '\0'; // We replace the '=' splitter by a null ending char
				name = sCell;
				value = &sCell[(j + 1)];
				break;
			}
		}

		if (name != NULL) {
			if (set)
				m2mp_client_settings_set_value_actual(this, name, value);

			if (get) {
				char * value = m2mp_client_settings_get_value(this, name);
				if (value) {
					size_t len = strlen(name) + strlen(value) + 2;
					char * sGetCell = (char *) malloc(sizeof (char) * len);
					sprintf(sGetCell, "%s=%s", name, value);
					linkedlist_insert_last(getCellsList, sGetCell);
				}
			}
		}

		free(sCell);
	}

	if (getAll) {
		dictionnary_entry ** array = (dictionnary_entry **) dictionnary_get_all(& this->settings);
		dictionnary_entry ** ptr = array;
		for (; *ptr; ptr++) {
			char * name = (*ptr)->name;
			char * value = (*ptr)->value;

			size_t len = strlen(name) + strlen(value) + 2;
			char * sGetCell = (char *) malloc(sizeof (char) * len);
			sprintf(sGetCell, "%s=%s", name, value);

			linkedlist_insert_last(getCellsList, sGetCell);
		}
		free(array);
	}

	if (getCellsList != NULL) {

		char ** getCellsStrArray = (char **) linkedlist_get_array(getCellsList);
		//mw_add( getCellsStrArray, __FILE__, __LINE__, __FUNCTION__ );

		m2mp_client_send_string_array(this->client, "_set", (const char **) getCellsStrArray);

		free(getCellsStrArray);

		linkedlist_empty(getCellsList, free);
		//mw_rmv(getCellsList);
		linkedlist_delete(& getCellsList);
	}

	m2mp_client_settings_save_if_necessary(this);
}

static void m2mp_client_settings_might_report_changes(m2mp_client_settings * this) {
	if (this->identified) { // If we are identified

		// We find all the settings that are left to be sent
		dictionnary_entry ** entries = (dictionnary_entry**) dictionnary_get_all(& this->settings);

		int i = 0;
		while (entries[i]) {
			dictionnary_entry * entry = entries[i];
			char * name = entry->name;
			char * value = entry->value;
			if (name[0] != '.' && name[0] != '_') {
				char modified_key[BUFSIZ];
				sprintf(modified_key, ".%s.modified", name);
				char buffer[BUFSIZ];
				sprintf(buffer, "%s=%s", name, value);
				if (m2mp_client_settings_get_value(this, modified_key)) {
					const char *array[] = {
						"g",
						buffer,
						NULL
					};
					m2mp_client_send_string_array(this->client, "_set", array);
				}
			}
			i++;
		}

		this->settingsChangeAckNb = m2mp_client_send_ack_request(this->client);

		free(entries);
	}
}

m2mp_client_event ** m2mp_client_settings_work(void * ptrClient, void * ptrThis, m2mp_client_event * event) {
	//m2mp_client * client = (m2mp_client *) ptrClient;
	m2mp_client_settings * this = (m2mp_client_settings *) ptrThis;

	if (!event)
		return NULL;

	if (event->type == M2MP_CLIENT_EVENT_DATA) {
		m2mp_client_event_data * dataEvent = (m2mp_client_event_data *) event;
		if (!strcmp(dataEvent->channelName, "_set")) {
			m2mp_client_settings_treat_data(this, dataEvent);
		}
	} else if (event->type == M2MP_CLIENT_EVENT_DATAARRAY) {
		m2mp_client_event_dataarray * dataArrayEvent = (m2mp_client_event_dataarray *) event;
		if (!strcmp(dataArrayEvent->channelName, "_set")) {
			m2mp_client_settings_treat_data_array(this, dataArrayEvent);
		}
	} else if (event->type == M2MP_CLIENT_EVENT_CONNECTED) {
		this->settingsChangeAckNb = -1;
	} else if (event->type == M2MP_CLIENT_EVENT_DISCONNECTED) {
		this->identified = false;
	} else if (event->type == M2MP_CLIENT_EVENT_IDENTIFIED_RESPONSE) {
		m2mp_client_event_identified * identified = (m2mp_client_event_identified *) event;

		this->identified = identified->status;

		m2mp_client_settings_might_report_changes(this);

	} else if (event->type == M2MP_CLIENT_EVENT_ACK_RESPONSE) {
		m2mp_client_event_ack_response * ack_response = (m2mp_client_event_ack_response *) event;
		if (ack_response->ackNb == this->settingsChangeAckNb) {
			// We delete all the entries that are left to be sent
			dictionnary_entry ** entries = (dictionnary_entry**) dictionnary_get_all(& this->settings);

			int i = 0;
			while (entries[i]) {
				dictionnary_entry * entry = entries[i];
				char * name = entry->name;
				if (name[0] != '.' && name[0] != '_') {
					char modified_key[BUFSIZ];
					sprintf(modified_key, ".%s.modified", name);
					if (m2mp_client_settings_get_value(this, modified_key)) {
						m2mp_client_settings_del(this, modified_key);
					}
				}
				i++;
			}
			free(entries);
		}
	}
	// Whatever happens, we don't change the event chain
	return NULL;
}

dictionnary_entry * m2mp_client_settings_get_entry(m2mp_client_settings * this, const char * name) {
	return dictionnary_get_entry(& this->settings, name);
}

unsigned char m2mp_client_settings_del(m2mp_client_settings * this, char * name) {
	return dictionnary_rmv(& this->settings, name);
}

void m2mp_client_settings_set_value_impl(m2mp_client_settings * this, char * name, char * value) {
	LOG(LVL_DEBUG, "m2mp_client_settings_set_value_impl( \"%s\", \"%s\" );", name, value);

	dictionnary_put(& this->settings, name, value);

	m2mp_client_settings_show_entries(this);
}

void m2mp_client_settings_set_value_actual(m2mp_client_settings * this, char * name, char * value) {
	LOG(LVL_DEBUG, "m2mp_client_settings_set_value( \"%s\", \"%s\" );", name, value);

	m2mp_client_settings_set_value_impl(this, name, value);

	this->modified = 1;
}

void m2mp_client_settings_set_value(m2mp_client_settings * this, char * name, char * value) {

	if (name[0] != '.' && name[0] != '_') { // We will check if it's a setting change
		char * previous = m2mp_client_settings_get_value(this, name);
		if (strcmp(value, previous)) {
			char buffer[BUFSIZ];
			sprintf(buffer, ".%s.modified", name);
			m2mp_client_settings_set_value_actual(this, buffer, "1");

			// Actual saving 
			m2mp_client_settings_set_value_actual(this, name, value);

			// We disable a previous acknowledge request we might have issued
			this->settingsChangeAckNb = -1;
			
			m2mp_client_settings_might_report_changes(this);
		}
	}
}

char * m2mp_client_settings_get_value(m2mp_client_settings * this, char * name) {
	return dictionnary_get_value(& this->settings, name);
}

void m2mp_client_settings_show_entries(m2mp_client_settings * this) {
	dictionnary_entry ** entries = dictionnary_get_all(& this->settings);
	//mw_add( entries, __FILE__, __LINE__, __FUNCTION__ );

	void * ptrToFree = (void *) entries;

	while (*entries) {
		dictionnary_entry * entry = *entries;
		LOG(LVL_DEBUG, "settings[\"%s\"]=\"%s\"", entry->name, entry->value);
		entries++;
	}

	free(ptrToFree);
}

void m2mp_client_settings_save(m2mp_client_settings* this) {
	LOG(LVL_DEBUG, "m2mp_client_settings_save( [...] );");
	FILE * fp = fopen("settings.txt", "wt");
	fprintf(fp, "# M2MP Config file\n");

	dictionnary_entry ** entries = (dictionnary_entry**) dictionnary_get_all(& this->settings);

	int i = 0;
	while (entries[i]) {
		dictionnary_entry * entry = entries[i];
		fprintf(fp, "%s=%s\n", entry->name, entry->value);
		i++;
	}

	free(entries);

	fclose(fp);

	this->modified = 0;
}

void m2mp_client_settings_load(m2mp_client_settings* this) {
	LOG(LVL_DEBUG, "m2mp_client_settings_load();");
	size_t bufferSize = 1024;
	char * buffer = malloc(sizeof ( char) * (bufferSize + 1));

	// We empty everything that could exist
	// linkedlist_empty(& this->settings, settings_entry_delete_void);
	dictionnary_clear(& this->settings);


	FILE * fp = fopen("settings.txt", "rt");
	if (!fp) {
		LOG(LVL_WARNING, "Settings file doesn't exist yet...");
		return;
	}
	int i;
	while (!feof(fp)) {
		char * line = fgets(buffer, bufferSize, fp);
		if (line) {
			line[ strlen(line) - 1 ] = '\0'; // We remove the trailing "\n"

			if (line[0] == '#') // We don't treat comments
				continue;

			for (i = 0; line[i]; i++) {
				if (line[i] == '=') {
					line[i] = '\0'; // We replace the '=' splitter by a null ending char
					m2mp_client_settings_set_value_impl(this, line, &line[(i + 1)]);
					break;
				}
			}
		}
	}

	fclose(fp);

	free(buffer);

	m2mp_client_settings_show_entries(this);
}

void m2mp_client_settings_save_if_necessary(m2mp_client_settings * this) {
	if (this->modified)
		m2mp_client_settings_save(this);
}
