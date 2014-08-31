#ifndef M2MP_CLIENT_SETTINGS_H
#define	M2MP_CLIENT_SETTINGS_H

#include <stdbool.h>

#include "m2mp_client.h"
#include "dictionnary.h"

#define TRUE 1
#define FALSE 0

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct st_m2mp_client_settings {
        m2mp_client *client;
        dictionnary settings;
        unsigned char modified;
		bool identified;
		int settingsChangeAckNb;
    } m2mp_client_settings;

    m2mp_client_settings * m2mp_client_settings_new(m2mp_client * client );

    void m2mp_client_settings_init(m2mp_client_settings * this, m2mp_client * client );

    m2mp_client_event ** m2mp_client_settings_work(void * ptrClient, void * ptrThis, m2mp_client_event * event);

    void m2mp_client_settings_delete(m2mp_client_settings * this);

    char * m2mp_client_settings_get_value(m2mp_client_settings * this, char * name);

    void m2mp_client_settings_set_value_actual(m2mp_client_settings * this, char * name, char * value);
	
	void m2mp_client_settings_set_value(m2mp_client_settings * this, char * name, char * value);
	
	unsigned char m2mp_client_settings_del(m2mp_client_settings * this, char * name);

    void m2mp_client_settings_show_entries(m2mp_client_settings * this );

    void m2mp_client_settings_save( m2mp_client_settings * this );

    void m2mp_client_settings_load( m2mp_client_settings * this );

    void m2mp_client_settings_save_if_necessary( m2mp_client_settings * this );

#ifdef	__cplusplus
}
#endif

#endif	/* M2MP_CLIENT_SETTINGS_H */

