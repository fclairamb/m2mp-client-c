/* 
 * File:   m2mp_client.h
 * Author: Florent
 *
 * Created on 22 janvier 2011, 00:58
 */

#ifndef M2MP_CLIENT_H
#define	M2MP_CLIENT_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef M2MP_CLIENT_TYPE
#define M2MP_CLIENT_TYPE
	typedef void m2mp_client;
#endif

#include "m2mp_client_types.h"

// History:
// 2011-01-22 - 0.0.1 : First version release
// 2012-04-12 - 0.1.0 : Added some documentation and a version method
// 2014-08-30 - 0.2.0 : Various (but small) changes

	/**
	 * Create a new m2mp_client_data instance
	 * @return m2mp_client instance
	 */
	m2mp_client * m2mp_client_new();
	
	/**
	 * Get the version of the M2MP client
     * @return A newly allocated string
     */
	char* m2mp_client_get_version();
	
	/**
	 * Delete a m2mp_client instance
	 * @param this m2mp_client_data to delete
	 */
	void m2mp_client_delete(m2mp_client * * this);

	/**
	 * Set the identifier of the client
	 * @param this m2mp_client instance
	 * @param ident Identifier
	 */
	void m2mp_client_set_ident(m2mp_client * this, const char * ident);

	/**
	 * Set the capabilities of the client
	 * @param this m2mp_client instance
	 * @param capabilities Capacibilites (ending with a NULL char*)
	 */
	void m2mp_client_set_capabilities(m2mp_client * this, const char ** capabilities);
	
	/**
	 * Set the status value of the client
     * @param this Instance of the client
     * @param name Name of the status (will be copied)
     * @param value Vaue of the status (will be copied)
     */
	void m2mp_client_set_status( m2mp_client * this, const char * name, const char * value );

	/**
	 * Open a connection to a host using the m2mp_client we created
	 * @param hostname Hostname to connect to
	 * @param port Port to connect to
	 */
	int m2mp_client_connect(m2mp_client * this, const char * hostname, int port);


	/**
	 * Discocnnect the client from the server
	 * @param this Client instance
	 */
	void m2mp_client_disconnect(m2mp_client * this);

	/**
	 * Does all the work
	 * @param this m2mp_client instance
	 * @param timeout max time allocated to wait for any network data
	 * @return m2mp_client_event instance or NULL
	 * 
	 * The returned m2mp_client_event must be deleted by m2mp_client_event_delete
	 */
	m2mp_client_event * m2mp_client_work(m2mp_client * this, int timeout /* = 500 */);

	/**
	 * Delete a new m2mp_client_event instance
	 * @param this m2mp_client_event to delete
	 */
	void m2mp_client_event_delete(m2mp_client * ins /* = NULL */, m2mp_client_event * this);

	/**
	 * Get the state of the client
	 * @param this m2mp_client instance
	 * @return m2mp_client_state
	 * @deprecated This will be remode
	 */
	m2mp_client_state m2mp_client_get_state(m2mp_client * this);

	/**
	 * Send some data on a named channel
	 * @param channel_name Named channel
	 * @param data Data to send
	 */
	void m2mp_client_send_data(m2mp_client * this, const char * channel_name, const unsigned char * data, size_t length);

	/**
	 * Send some data array on a named channel
	 * @param channel_name Named channel
	 * @param data Data to send
	 */
	void m2mp_client_send_data_array(m2mp_client * this, const char * channel_name, const unsigned char ** data, size_t *lengths);

	/**
	 * Send a string
	 * @param this m2mp_client instance
	 * @param channel_name Named channel to use
	 * @param string Text to send
	 */
	void m2mp_client_send_string(m2mp_client * this, const char * channel_name, const char * string);

	/**
	 * Send an array of strings
	 * @param this m2mp_client instance
	 * @param channel_name Name channel to use
	 * @param string Text to send
	 */
	void m2mp_client_send_string_array(m2mp_client * this, const char * channel_name, const char ** string);


	/**
	 * Send an acknowledge request
	 * @param this m2mp_client instance
	 * @return Ack number sent
	 */
	int m2mp_client_send_ack_request(m2mp_client * this);

	/**
	 * Send an acknowledge request with a particular acknowledge number
	 * @param this m2mp_client instance
	 * @param ackNb acknowledge number
	 */
	void m2mp_client_send_ack_requestNb(m2mp_client * this, unsigned char ackNb);

	/**
	 * Send an acknowledge response
	 * @param this m2mp_client instance
	 * @param ackNb acknowledge number of the response
	 */
	void m2mp_client_send_ack_response(m2mp_client * this, unsigned char ackNb);

	/**
	 * Convert an m2mp_client_event_data into a string
	 * @param this m2mp_client_event_data instance
	 * @return string (newly allocated)
	 */
	char * m2mp_client_event_data_get_string(m2mp_client_event_data * this);

	/**
	 * Convert a string from a m2mp_client_event_dataarray 
	 * @param this m2mp_client_event_dataarray instance
	 * @param i Index of the string to get within the array
	 * @return String at the corresponding index array (newly allocated)
	 */
	char * m2mp_client_event_dataarray_get_string(m2mp_client_event_dataarray * this, int i);

	/**
	 * Register a plugin to the M2MP library
	 * @param this Instance of the library
	 * @param instance Instance of the plugin to register
	 * @param func method to call to process future events
	 */
	void m2mp_client_add_plugin(m2mp_client * this, void * instance, m2mp_client_event ** (*func)(void * clientInstance, void * pluginInstance, m2mp_client_event * event));

	/**
	 * Deregister a plugin from the M2MP library
	 * @param this Instance of the library
	 * @param instance Instance of the plugin
	 */
	void m2mp_client_rmv_plugin(m2mp_client * this, void * instance);

	/**
	 * Register a plugin event type ID
     * @param this Instance of the m2mp client library
     * @return event type ID
     */
	int m2mp_client_register_event_type( m2mp_client * this );
	
	/**
	 * Register a destructor for an event type ID
     * @param this Instance of the m2mp client library
     * @param event_id Event id
     * @param func Destructor to uses
     */
	void m2mp_client_register_event_destructor( m2mp_client * this, int event_id, void (*func)(m2mp_client_event * event)  );
#ifdef	__cplusplus
}
#endif

#endif	/* M2MP_CLIENT_H */

