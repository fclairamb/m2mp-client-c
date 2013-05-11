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
#include "memwatcher.h"


m2mp_client_settings * m2mp_client_settings_new(m2mp_client * client) {
    m2mp_client_settings * this = (m2mp_client_settings *) mw_malloc(sizeof (m2mp_client_settings));

    m2mp_client_settings_init(this, client);

    m2mp_client_settings_load(this);

    return this;
}

void m2mp_client_settings_init(m2mp_client_settings * this, m2mp_client * client) {
    this->modified = 0;
    this->client = client;
	dictionnary_init(& this->settings );
	
	m2mp_client_add_plugin(client, this, m2mp_client_settings_work);
}

void m2mp_client_settings_delete(m2mp_client_settings ** pThis) {
    m2mp_client_settings * this = *pThis;

	dictionnary_clear( & this->settings );

    mw_free(this);
    *pThis = NULL;
}

void m2mp_client_settings_treat_data(m2mp_client_settings * this, m2mp_client_event_data *dataEvent) {
    char * str = m2mp_client_event_data_get_string(dataEvent);
    LOG(LVL_DEBUG, "data = \"%s\"", str);
    mw_free(str);
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

        mw_free(sFirstCell);
    }

    linkedlist * getCellsList = NULL;

    if (get) {
        getCellsList = linkedlist_new();
        //mw_add(getCellsList, __FILE__, __LINE__, __FUNCTION__);
        linkedlist_insert_last(getCellsList, str_clone("g"));
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
                m2mp_client_settings_set_value(this, name, value);

            if (get) {
                char * value = m2mp_client_settings_get_value(this, name);
                if (value) {
                    size_t len = strlen(name) + strlen(value) + 2;
                    char * sGetCell = (char *) mw_malloc(sizeof (char) * len);
                    sprintf(sGetCell, "%s=%s", name, value);
                    linkedlist_insert_last(getCellsList, sGetCell);
                }
            }
        }

        mw_free(sCell);
    }

    if (getAll) {
        dictionnary_entry ** array = (dictionnary_entry **) dictionnary_get_all(& this->settings);
        for (; array; array++) {
            char * name = (*array)->name;
            char * value = (*array)->value;

            size_t len = strlen(name) + strlen(value) + 2;
            char * sGetCell = (char *) mw_malloc(sizeof (char) * len);
            sprintf(sGetCell, "%s=%s", name, value);
            
            linkedlist_insert_last(getCellsList, sGetCell);
        }
        mw_free( array );
    }

    if (getCellsList != NULL) {

        char ** getCellsStrArray = (char **) linkedlist_get_array(getCellsList);
        //mw_add( getCellsStrArray, __FILE__, __LINE__, __FUNCTION__ );

        m2mp_client_send_string_array(this->client, "_set", (const char **) getCellsStrArray);

        mw_free(getCellsStrArray);

        linkedlist_empty(getCellsList, mw_free);
        //mw_rmv(getCellsList);
        linkedlist_delete(& getCellsList);
    }

    m2mp_client_settings_save_if_necessary(this);
}

m2mp_client_event ** m2mp_client_settings_work(void * ptrClient, void * ptrThis, m2mp_client_event * event) {
    //m2mp_client * client = (m2mp_client *) ptrClient;
    m2mp_client_settings * this = (m2mp_client_settings *) ptrThis;

	if ( ! event )
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
    }

	// Whatever happens, we don't change the event chain
    return NULL;
}

dictionnary_entry * m2mp_client_settings_get_entry(m2mp_client_settings * this, const char * name) {
	return dictionnary_get_entry( & this->settings, name );
}

unsigned char m2mp_client_settings_rmv(m2mp_client_settings * this, char * name) {
	dictionnary_rmv( & this->settings, name );
}

void m2mp_client_settings_set_value_impl(m2mp_client_settings * this, char * name, char * value) {
    LOG(LVL_DEBUG, "m2mp_client_settings_set_value_impl( \"%s\", \"%s\" );", name, value);
    
	dictionnary_put(& this->settings, name, value );

    m2mp_client_settings_show_entries(this);
}

void m2mp_client_settings_set_value(m2mp_client_settings * this, char * name, char * value) {
    LOG(LVL_DEBUG, "m2mp_client_settings_set_value( \"%s\", \"%s\" );", name, value);

    m2mp_client_settings_set_value_impl(this, name, value);

    this->modified = 1;
}

char * m2mp_client_settings_get_value(m2mp_client_settings * this, char * name) {
	return dictionnary_get_value( & this->settings, name );
}

void m2mp_client_settings_show_entries(m2mp_client_settings * this) {
	dictionnary_entry ** entries = dictionnary_get_all( & this->settings );
    //mw_add( entries, __FILE__, __LINE__, __FUNCTION__ );

	void * ptrToFree =  (void *) entries;
	
    while ( *entries ) {
		dictionnary_entry * entry = *entries;
        LOG(LVL_DEBUG, "settings[\"%s\"]=\"%s\"", entry->name, entry->value);
		entries++;
    }

    mw_free(ptrToFree);
}

void m2mp_client_settings_save(m2mp_client_settings* this) {
    LOG(LVL_DEBUG, "m2mp_client_settings_save( [...] );");
    FILE * fp = fopen("settings.txt", "wt");
    fprintf(fp, "# M2MP Config file\n");

    //settings_entry ** entries = (settings_entry **) linkedlist_get_array(& this->settings);
	dictionnary_entry ** entries = (dictionnary_entry**) dictionnary_get_all( & this->settings );
	//mw_add( entries, __FILE__, __LINE__, __FUNCTION__ );

    int i = 0;
    while (entries[i]) {
        dictionnary_entry * entry = entries[i];
        fprintf(fp, "%s=%s\n", entry->name, entry->value);
        i++;
    }

    mw_free(entries);

    fclose(fp);

    this->modified = 0;
}

void m2mp_client_settings_load(m2mp_client_settings* this) {
    LOG(LVL_DEBUG, "m2mp_client_settings_load();");
    size_t bufferSize = 1024;
    char * buffer = mw_malloc(sizeof ( char) * (bufferSize + 1));

    // We empty everything that could exist
	// linkedlist_empty(& this->settings, settings_entry_delete_void);
	dictionnary_clear( & this->settings );
	
	
    FILE * fp = fopen("settings.txt", "rt");
    if ( ! fp ) {
        LOG( LVL_WARNING, "Settings file doesn't exist yet...");
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

    mw_free(buffer);

    m2mp_client_settings_show_entries(this);
}

void m2mp_client_settings_save_if_necessary(m2mp_client_settings * this) {
    if (this->modified)
        m2mp_client_settings_save(this);
}
