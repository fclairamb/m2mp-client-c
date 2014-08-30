/* 
 * File:   m2mp_client_internal.h
 * Author: florent
 *
 * Created on 22 January 2011, 21:48
 */

#ifndef M2MP_CLIENT_INTERNAL_H
#define	M2MP_CLIENT_INTERNAL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <time.h>
#include <sys/poll.h>

#include "linkedlist.h"
#include "dictionnary.h"
#include "m2mp_client_types.h"

    /** BYTE type */
    typedef unsigned char BYTE;

    /** BOOL type */
    typedef unsigned char BOOL;

	/** Plugin entry */
    typedef struct st_m2mp_plugin_entry {
		/** An instance of a plugin */
        void * pluginInstance;
		
		/** A callback method to treat events */
        m2mp_client_event ** (*func)(void * clientInstance, void * pluginInstance, m2mp_client_event * event);
    } m2mp_client_plugin_entry;
	
	typedef struct st_m2mp_plugin_event_destructor {
		int eventId;
		void (*destructor)( m2mp_client_event * event );
	} m2mp_client_event_destructor;
	
	typedef struct st_m2mp_status_entry {
        char * name;
        char * value;
    } m2mp_client_status_entry;
	
	
	m2mp_client_status_entry * m2mp_client_status_entry_new( const char * name, const char * value );
	
	void m2mp_client_status_entry_delete( m2mp_client_status_entry * cs );
	
	

	/** Key data structure */
    struct st_m2mp_client {
        /** Client identification */
        char * ident;

        /** Client capabilities */
//        char ** capabilities;

        BOOL autoReplyToAckRequests;

        /** Map of the sending channels */
        char *sendChannels [256];

        /** Map of the receving channels */
        char *recvChannels [256];

        /** Socket file descriptor */
        int socketFD;

		/** Receiving buffer */
        unsigned char * recvBuffer;
		/** Receiving buffer size */
        size_t recvBufferSize;
		
		/** Receiving buffer position (current position of the reception) */
        size_t recvBufferPosition;
		/** Receiving buffer expected header size (we must allocate at least this size to know what size has the message) */
        size_t recvBufferExpectedHeaderSize;
		/** Receiving buffer expected frame size (we must allocate at least this size to get the frame) */
        size_t recvBufferExpectedFrameSize;

		/** Sending buffer */
		unsigned char * sendBuffer;
		/** Sending buffer size */
        size_t sendBufferSize;

		/**
		 * Last time some data was sent
		 */
        time_t lastSendTime;
		
		/**
		 * Last time some data was received
		 */
        time_t lastRecvTime;
		
		/**
		 * Last time we acknowledge some data
		 */
        time_t lastAckSent;

		/**
		 * Last ack number sent
		 */
        unsigned char ackNumber;

		/**
		 * Current state of the client
		 */
        unsigned char state;

		/** 
		 * Socket handling
		 */
        struct pollfd socketPollFDs;

		/**
		 * Linked list of all the events waiting to be processed
		 */
        linkedlist events;

		/**
		 * Linked list that contains all the plugins
		 */
        linkedlist pluginsLinkedList;

		/**
		 * Array of all the plugins. The only goal of this code
		 * is to allow all the plugins.
		 */
        m2mp_client_plugin_entry ** pluginsArray;
		
		/**
		 * All the statuses
		 */
		dictionnary statuses;
		
		/** Event types increment */
		unsigned int event_types;
		
		/** event type destructors */
		m2mp_client_event_destructor *event_destructors;
    };





#ifndef M2MP_CLIENT_TYPE
#define M2MP_CLIENT_TYPE
    typedef struct st_m2mp_client m2mp_client;
#endif

#include "m2mp_client.h"
#include "logging.h"


    /** Send identification message */
#define PROT_S_IDENT  0x01

    /** Receive dentification result message */
#define PROT_R_IDENT_RESULT  0x01

    /** Send acknowledge request */
#define PROT_S_ACK_REQUEST  0x02

    /** Send acknowledge request reponse */
#define PROT_S_ACK_RESPONSE  0x03

    /** Receive acknowledge response */
#define PROT_R_ACK_RESPONSE  0x02

    /** Receive acknowledge request */
#define PROT_R_ACK_REQUEST  0x03

    /** Send named channel definition */
#define PROT_S_NC_DEF  0x20

    /** Receive named channel definition */
#define PROT_R_NC_DEF  0x20

    /** Send data on a named channel */
#define PROT_S_NC_DATA  0x21

    /** Receive data on a named channel */
#define PROT_R_NC_DATA  0x21

    /** Send an array of data on a named channel */
#define PROT_S_NC_DATAARRAY  0x22

    /** Receive an array of data on a named channel */
#define PROT_R_NC_DATAARRAY  0x22

    /** Send a large data on a named channel */
#define PROT_S_NC_DATA_LARGE  0x41

    /** Receive a large data on a named channel */
#define PROT_R_NC_DATA_LARGE  0x41

    /** Send an array of large data on a named channel */
#define PROT_S_NC_DATAARRAY_LARGE  0x42

    /** Send an array of large data on a named channel */
#define PROT_R_NC_DATAARRAY_LARGE  0x42

    /** Send data on a named channel */
#define PROT_S_NC_DATA_VERYLARGE 0x61

    /** Receive data on a named channel */
#define PROT_R_NC_DATA_VERYLARGE 0x61

    /** Send an array of data on a named channel */
#define PROT_S_NC_DATAARRAY_VERYLARGE 0x62

    /** Receive an array of data on a named channel */
#define PROT_R_NC_DATAARRAY_VERYLARGE 0x62

    /** Max value of the special messages */
#define PROT_MSG_SPECIALIZED_MAX  0x20

    /** Max value of the 1 #define sized messages */
#define PROT_MSG_1BYTESIZED_MAX  0x40

    /** Max value of the 2 bytes sized messages */
#define PROT_MSG_2BYTESSIZED_MAX  0x60

    /** Max value of the 4 bytes sized messages */
#define PROT_MSG_4BYTESSIZED_MAX  0x80

    /**
     * Initialization of the m2mp_client
     * @param this Instance of the client
     */
    void m2mp_client_init(m2mp_client * this);

    //    /**
    //     * Handle data received from the network
    //     * @param this m2mp_client instance
    //     * @param data Data received from the network
    //     * @param length Length of the data received
    //     */
    //    void m2mp_client_net_received(m2mp_client * this, BYTE * data, size_t length);

    /**
     * Send data to the network
     * @param this m2mp_client instance
     * @param data Data to send to the network
     * @param length Length of the data to send
     */
    void m2mp_client_net_send(m2mp_client * this, BYTE * data, size_t length);

    int m2mp_client_connection_check(m2mp_client * this);

    int m2mp_client_receive(m2mp_client * this, int timeout /* = 500 */);

    int m2mp_client_treat_frame(m2mp_client * this, BYTE * data, size_t length);

    void m2mp_client_send_channel_id_and_name(m2mp_client * this, const unsigned char id, const char* name);

    void m2mp_client_send_identification(m2mp_client * this, const char * identification);

    void m2mp_client_debug_render_bytes(BYTE * data, size_t length);

    /**
     * Prepare the send bufer 
     * @param this m2mp_client instance
     * @param length Length of the m2mp_client
     */
    void m2mp_client_prepare_send_buffer(m2mp_client * this, size_t length);

    void m2mp_client_prepare_recv_buffer(m2mp_client * this, size_t length);

    /**
     * Init the list of channels
     * @param channels Channels to initialize
     */
    void m2mp_client_channels_init(char * channels[256]);

    /**
     * Get a channel name from a channel id
     * @param channels Channels array
     * @param id Id of the channel we want
     * @return Name of the channel
     */
    char * m2mp_client_channels_get_name(char * channels[256], const unsigned char id);

    /**
     * Get a channel id from a channel name
     * @param channels Channels array
     * @param name Name of the channel
     * @return Id of the channel
     */
    int m2mp_client_channels_get_id(m2mp_client * this, char * channels[256], const char * name);

    /**
     * Set the name of a given channel id
     * @param channels Channels to work on
     * @param id Id of the channel
     * @param name Name of the channel
     */
    void m2mp_client_channels_set_id_and_name(char * channels[256], const unsigned char id, const char * name);


    void m2mp_client_add_event(m2mp_client * this, m2mp_client_event* event);

    /**
     * Liberate the channels
     * @param channels Channels to
     */
    void m2mp_client_channels_free(char * channels[256]);

    /**
     * Delete a m2mp_client_event instance (using a void * pointer)
     * @param this m2mp_client_event to delete
     */
    void m2mp_client_event_delete_void(void * this);

    /**
     * Get events generated by the library
     * @param this m2mp_client instance
     * @return m2mp_client_event instance
     */
    m2mp_client_event * m2mp_client_get_event(m2mp_client * this);

    void m2mp_client_update_plugin_array(m2mp_client * this);

#ifdef	__cplusplus
}
#endif

#endif	/* M2MP_CLIENT_INTERNAL_H */

