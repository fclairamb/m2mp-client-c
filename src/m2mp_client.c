#include "m2mp_client_internal.h"
#include "dictionnary.h"
#include "str.h"

// Standard things
#include <stdio.h>
#include <unistd.h>
#include <endian.h>
#ifndef htobe16
#include "_endian_.h"
#endif

// Network
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>

#define M2MP_CLIENT_VERSION "0.2.0"

m2mp_client_status_entry * m2mp_client_status_entry_new( const char * name, const char * value ) {
	m2mp_client_status_entry * entry = malloc( sizeof(m2mp_client_status_entry ) );
	
	entry->name = strdup( name );
	entry->value = strdup( value );
	
	return entry;
}

void m2mp_client_status_entry_delete( m2mp_client_status_entry * entry ) {
	free( entry->name );
	free( entry->value );
}

void m2mp_client_status_entry_delete_void( void * entry ) {
	m2mp_client_status_entry_delete( (m2mp_client_status_entry*) entry );
}



m2mp_client * m2mp_client_new() {
    LOG( LVL_DEBUG, "m2mp_client_new();");
    m2mp_client * this = malloc(sizeof ( m2mp_client));

    m2mp_client_init(this);

    return this;
};

char * m2mp_client_get_version() {
	return M2MP_CLIENT_VERSION;
}

void m2mp_client_set_ident(m2mp_client* this, const char* ident) {
    if ( this->ident )
        free( this->ident );

    char * ident_copy = (char *) malloc(strlen(ident) + 1);
    strcpy(ident_copy, ident);
    this->ident = ident_copy;
}

void m2mp_client_set_capabilities(m2mp_client * this, const char ** capabilities) {
	const char * sep = ",";
	char * value = str_array_implode(capabilities, sep);
	m2mp_client_set_status(this, "cap", value );
	free( value );
}

void m2mp_client_set_status( m2mp_client * this, const char * name, const char * value ) {
	dictionnary_put( & this->statuses, name, value );
}

void m2mp_client_event_delete_void( void * this ) {
    m2mp_client_event_delete(NULL, (m2mp_client_event * ) this );
}

void m2mp_client_plugin_event_delete(m2mp_client * this /* = NULL */, m2mp_client_event* event) {
	this->event_destructors[ event->type - M2MP_CLIENT_EVENT_PLUGINS ].destructor( event );
}

void m2mp_client_event_delete(m2mp_client * this /* = NULL */, m2mp_client_event* event) {

    if ( event == NULL )
        return;

    switch( event->type) {

        case M2MP_CLIENT_EVENT_DATA: {
            m2mp_client_event_data * eventData = (m2mp_client_event_data *) event;
            free( eventData->channelName );
            free( eventData->data );
            break;
        }

        case M2MP_CLIENT_EVENT_DATAARRAY: {
            m2mp_client_event_dataarray * eventDataArray = (m2mp_client_event_dataarray *) event;
            int i = 0;
            free( eventDataArray->channelName );
            for( i = 0; eventDataArray->data[i]; ++i )
                free( eventDataArray->data[i] );
            free( eventDataArray->data );
            free( eventDataArray->lengths );
            break;
        }

        case M2MP_CLIENT_EVENT_CONNECTED: {
            m2mp_client_event_connected * eventConnected = (m2mp_client_event_connected *) event;
            free( eventConnected->serverHostname );
        }
            break;
        case M2MP_CLIENT_EVENT_IDENTIFIED_RESPONSE:
        case M2MP_CLIENT_EVENT_DISCONNECTED:
        case M2MP_CLIENT_EVENT_ACK_RESPONSE:
        case M2MP_CLIENT_EVENT_ACK_REQUEST:
		case M2MP_CLIENT_EVENT_POLL_ERROR:
            break;

        default:
			if ( event->type >= M2MP_CLIENT_EVENT_PLUGINS) {
				m2mp_client_plugin_event_delete(this, event);
				return;
			}
			else {
				LOG( LVL_CRITICAL, "ERROR: event->type = %d is NOT handled !", event->type );
			}
    }

    free( event );
}

void m2mp_client_delete(m2mp_client ** pThis) {
    m2mp_client * this = *pThis;
    LOG(LVL_DEBUG, "m2mp_client_delete( m2mp_client{0x%08X} );", (unsigned int) (long unsigned int) this );
    if (this) {

        m2mp_client_disconnect(this);

        // We free the channels
        m2mp_client_channels_free(this->recvChannels);
        m2mp_client_channels_free(this->sendChannels);

        // We free the receiving buffer
        if (this->recvBuffer)
            free(this->recvBuffer);

        // We free the sending buffer
        if (this->sendBuffer)
            free(this->sendBuffer);

        if ( this->ident )
            free( this->ident );

        linkedlist_empty( & this->events, m2mp_client_event_delete_void );

		dictionnary_clear( & this->statuses );
		
        linkedlist_empty( & this->pluginsLinkedList, free );

        if ( this->pluginsArray )
            free( this->pluginsArray );
		
		free( this->event_destructors );
		
        free(this);
        *pThis = NULL;
    }
}

void m2mp_client_received_packet_processed( m2mp_client * this) {
	this->recvBufferPosition = 0;
	this->recvBufferExpectedHeaderSize = 0;
	this->recvBufferExpectedFrameSize = 0;
}

void m2mp_client_init_channels_array( char **array ) {
	int i = 0;
	for( i = 0; i < 256; i++ ) {
		array[i] = NULL;
	}
}

/**
 * This codes act both for initial setup and disconnection
 * @param this m2mp_client instance
 */
void m2mp_client_init_common( m2mp_client * this ) {
	    this->socketFD = -1;

    this->lastRecvTime = time(NULL);
    this->lastSendTime = time(NULL);


    this->recvBufferPosition = 0;
    this->recvBufferExpectedFrameSize = 0;

    this->ackNumber = 0;
	
	m2mp_client_received_packet_processed( this );
}

/**
 * This codes act in case of disconnection (Connection context lost)
 * @param this
 */
void m2mp_client_init_disconnected( m2mp_client * this ) {
	

    m2mp_client_channels_free(this->recvChannels);
    m2mp_client_channels_free(this->sendChannels);

    this->state = M2MP_CLIENT_STATE_NOT_CONNECTED;
	
	m2mp_client_init_common(this);
}

void m2mp_client_init(m2mp_client * this) {

	// The buffers are initialized to a NULL value
    this->recvBuffer = NULL;
	this->recvBufferSize = 0;
    this->sendBuffer = NULL;
	this->sendBufferSize = 0;

	// We prepare the buffers
    m2mp_client_prepare_send_buffer( this, 1024 );
    m2mp_client_prepare_recv_buffer( this, 1024 );

	// We init the channels array
	m2mp_client_init_channels_array( this->recvChannels );
	m2mp_client_init_channels_array( this->sendChannels );
	
    this->ident = NULL;

    this->autoReplyToAckRequests = 1;
    
    linkedlist_init( & this->events );

    linkedlist_init( & this->pluginsLinkedList );
	
	dictionnary_init( & this->statuses );

    this->pluginsArray = NULL;
	
	this->event_types = M2MP_CLIENT_EVENT_PLUGINS;
	
	this->event_destructors = (m2mp_client_event_destructor *) malloc( 16 );
	
	m2mp_client_init_common(this);
}

m2mp_client_state m2mp_client_get_state(m2mp_client* this) {
    return this->state;
}

/*
void m2mp_client_net_received(m2mp_client *this, BYTE * data, size_t length) {


}
*/

int m2mp_client_connect(m2mp_client * this, const char * hostname, int port) {
    LOG( LVL_DEBUG, "m2mp_client_connect( \"%s\", %d );", hostname, port );

    struct sockaddr_in stSockAddr;
    this->socketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (this->socketFD == -1) {
        LOG( LVL_CRITICAL,"ERROR: Cannot create socket");

        m2mp_client_event_disconnected * event = (m2mp_client_event_disconnected *) malloc(sizeof ( m2mp_client_event_disconnected));
        event->base.type = M2MP_CLIENT_EVENT_DISCONNECTED;
        m2mp_client_add_event(this, (m2mp_client_event *) event);

        return EXIT_FAILURE;
    }

    memset(&stSockAddr, 0, sizeof ( stSockAddr));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(3000);

    struct hostent * he;

    if ((he = gethostbyname(hostname)) == NULL) {
        LOG( LVL_CRITICAL,"ERROR: Host could not be resolved.");
        this->socketFD = -1;

        m2mp_client_event_disconnected * event = (m2mp_client_event_disconnected *) malloc(sizeof ( m2mp_client_event_disconnected));
        event->base.type = M2MP_CLIENT_EVENT_DISCONNECTED;
        m2mp_client_add_event(this, (m2mp_client_event *) event);

        return EXIT_FAILURE;
    }

    struct in_addr **addr_list = (struct in_addr **) he->h_addr_list;
    int res = 0;

    int i;
    for (i = 0; addr_list[i] != NULL; ++i) {
        LOG( LVL_DEBUG,"Address: %s", inet_ntoa(*addr_list[i]));

        res = inet_pton(AF_INET, inet_ntoa(*addr_list[i]), & stSockAddr.sin_addr);

        if (res > 0) {
            break;
        } else if (res < 0) {
            LOG( LVL_CRITICAL,"ERROR: Unvalid address family.");
        } else if (res == 0) {
            LOG( LVL_CRITICAL,"ERROR: Unvalid IP address.");
        }
    }

    if (res <= 0) {
		LOG( LVL_CRITICAL, "ERROR: No address could be found !");
        close(this->socketFD);
        this->socketFD = -1;

        m2mp_client_event_disconnected * event = (m2mp_client_event_disconnected *) malloc(sizeof ( m2mp_client_event_disconnected));
        event->base.type = M2MP_CLIENT_EVENT_DISCONNECTED;
        m2mp_client_add_event(this, (m2mp_client_event *) event);

        return EXIT_FAILURE;
    }

/*
    setsockopt( this->socketFD, SOL_SOCKET, SO_RCVBUF, this->rawSockerRecvBuffer, 1024 );
*/

    if (connect(this->socketFD, (struct sockaddr *) & stSockAddr, sizeof ( stSockAddr)) == -1) {
        LOG( LVL_CRITICAL,"ERROR: Connect to server failed.");
        close(this->socketFD);
        this->socketFD = -1;

        m2mp_client_event_disconnected * event = (m2mp_client_event_disconnected *) malloc(sizeof ( m2mp_client_event_disconnected));
        event->base.type = M2MP_CLIENT_EVENT_DISCONNECTED;
        m2mp_client_add_event(this, (m2mp_client_event *) event);

        return EXIT_FAILURE;
    }

    this->state = M2MP_CLIENT_STATE_CONNECTED;

    m2mp_client_send_identification( this, this->ident );

    this->socketPollFDs.fd = this->socketFD;
    this->socketPollFDs.events = POLLIN | POLLPRI | POLLERR | POLLHUP;

    { // We create the connected event
        m2mp_client_event_connected * event = (m2mp_client_event_connected *) malloc( sizeof(m2mp_client_event_connected) );
        event->base.type = M2MP_CLIENT_EVENT_CONNECTED;
        event->serverHostname = strdup( hostname );
        event->serverPort = port;
        m2mp_client_add_event( this, (m2mp_client_event *) event );
    }

    return EXIT_SUCCESS;
}

void m2mp_client_disconnect(m2mp_client* this) {
    if ( this->socketFD != -1) {
        shutdown(this->socketFD, SHUT_RDWR);
        close(this->socketFD);
        this->socketFD = -1;
    }
}

void m2mp_client_send_identification( m2mp_client * this, const char * identification ) {
    LOG( LVL_DEBUG, "m2mp_client_send_identification( \"%s\" );", identification );
    size_t len = strlen( identification );

    m2mp_client_prepare_send_buffer( this, len + 2 );
    BYTE * buffer = this->sendBuffer;

    buffer[0] = PROT_S_IDENT;
    buffer[1] = (unsigned char ) len;
    memcpy( & buffer[2], identification, len );

    m2mp_client_net_send( this, buffer, len + 2);
}

void m2mp_client_send_channel_id_and_name(m2mp_client * this, const unsigned char id, const char* name) {
    LOG( LVL_DEBUG,"m2mp_client_send_channel_id_and_name( %d, \"%s\" );", (int) id, name );
    size_t len = strlen( name );

    m2mp_client_prepare_send_buffer( this, len + 3 );
    BYTE * buffer = this->sendBuffer;
    buffer[0] = PROT_S_NC_DEF;
    buffer[1] = (unsigned char) len + 1;
    buffer[2] = id;
    memcpy( & buffer[3], name, len );

    m2mp_client_net_send( this, buffer, len + 3 );
}

void m2mp_client_send_ack_requestNb(m2mp_client* this, unsigned char ackNb) {
    LOG( LVL_VERBOSE,"m2mp_client_send_ack_request( %d );", ackNb );
    m2mp_client_prepare_send_buffer( this, 2 );

    BYTE * buffer = this->sendBuffer;
    buffer[0] = PROT_S_ACK_REQUEST;
    buffer[1] = ackNb;

    this->lastAckSent = time(NULL);
    m2mp_client_net_send( this, buffer, 2 );
}

void m2mp_client_send_ack_request(m2mp_client* this) {
    m2mp_client_send_ack_requestNb( this, this->ackNumber++ );
}

void m2mp_client_send_ack_response(m2mp_client* this, unsigned char ackNb) {
    LOG( LVL_VERBOSE,"m2mp_client_send_ack_response( %d );", ackNb );
    m2mp_client_prepare_send_buffer( this, 2 );

    BYTE * buffer = this->sendBuffer;
    buffer[0] = PROT_S_ACK_RESPONSE;
    buffer[1] = ackNb;

    m2mp_client_net_send( this, buffer, 2 );
}

void m2mp_client_send_data(m2mp_client* this, const char* channel_name, const unsigned char* data, size_t length) {
    LOG( LVL_VERBOSE,"m2mp_client_send_data( \"%s\", ", channel_name );
    if ( DO_I_LOG( LVL_VERBOSE ) ) {
        m2mp_client_debug_render_bytes( (BYTE*) data, length );
		printf(" );");
		fflush(stdout);
	}

    int channelId = m2mp_client_channels_get_id(this, this->sendChannels, channel_name);

    size_t offset = 0;

    BYTE * buffer;

    // 0 to 254 bytes
    if (length <= 254) {
        m2mp_client_prepare_send_buffer(this, length + 3);
        buffer = this->sendBuffer;

        buffer[offset++] = PROT_S_NC_DATA;
        buffer[offset++] = length + 1;
    }// 255 to 65534 bytes (more is currently not handled)
    else {
        m2mp_client_prepare_send_buffer(this, length + 4);
        buffer = this->sendBuffer;

        buffer[offset++] = PROT_S_NC_DATA_LARGE;
        unsigned int l = length + 1;
        buffer[offset++] = (l >> 8) & 0xFF;
        buffer[offset++] = l & 0xFF;
    }
    buffer[offset++] = (unsigned char) channelId;
    memcpy(& buffer[offset], data, length);

    offset += length;

    m2mp_client_net_send(this, buffer, offset);
}

size_t m2mp_client_get_data_array_total_size(const unsigned char ** data, size_t * lengths) {
    int size;
    size_t length = 0;
    for (size = 0; data[size]; ++size)
        length += lengths[size];

    size_t totalSize = 0;

    // Best case scenario...
    totalSize = length + size + 1 /* channel id */;

    // If it got too big
    if (totalSize > 254 /*|| size > 255*/)
        totalSize = length + 2 * size + 1 /* channel id */;

    if (totalSize > 65534 /*|| size > 65535*/)
        totalSize = length + 4 * size + 1 /* channel id */;

    return totalSize;
}

void m2mp_client_send_data_array(m2mp_client* this, const char* channel_name, const unsigned char** data, size_t* lengths) {
    LOG(LVL_DEBUG, "m2mp_client_send_data_array( \"%s\", [...], [...] );", channel_name);

    size_t messageSize = m2mp_client_get_data_array_total_size(data, lengths);
    size_t totalSize = 0;

    int channelId = m2mp_client_channels_get_id(this, this->sendChannels, channel_name);

    unsigned char frameType = 0;

	// The total size is : the type + the length header size + the messagesize
	// There was a huge bug that I discovered with vallgrind
    if (totalSize <= 255) {
        frameType = PROT_S_NC_DATAARRAY;
        totalSize = 1  + 1 + messageSize;
    } else if (totalSize <= 65535) {
        frameType = PROT_S_NC_DATAARRAY_LARGE;
        totalSize = 1 + 2 + messageSize;
    } else {
        frameType = PROT_S_NC_DATAARRAY_VERYLARGE;
        totalSize = 1 + 4 + messageSize ;
    }

    LOG(LVL_DEBUG, "messageSize: %d, totalSize: %d", (int) messageSize, (int) totalSize);

    BYTE * frame = malloc(sizeof (BYTE) * totalSize);
    size_t offset = 0;

    frame[offset++] = frameType;
    switch (frameType) {
        case PROT_S_NC_DATAARRAY:
            frame[offset++] = (unsigned char) messageSize;
            break;
        case PROT_S_NC_DATAARRAY_LARGE:
            * ((uint16_t*) (& frame[offset])) = htole16(messageSize);
            offset += 2;
            break;
        case PROT_S_NC_DATAARRAY_VERYLARGE:
            * ((uint32_t *) (& frame[offset])) = htole32(messageSize);
            offset += 4;
            break;
        default:
            assert(!"Message type not handled...");
    }

    frame[ offset++ ] = (unsigned char) channelId;

    int i;
    for (i = 0; data[i]; ++i) {
        size_t length = lengths[i];
        const unsigned char * subData = data[i];
        switch (frameType) {
            case PROT_S_NC_DATAARRAY:
                frame[offset++] = (unsigned char) length;
                break;
            case PROT_S_NC_DATAARRAY_LARGE:
                * ((uint16_t*) (& frame[offset])) = htole16(length);
                offset += 2;
                break;
            case PROT_S_NC_DATAARRAY_VERYLARGE:
                * ((uint32_t *) (& frame[offset])) = htole32(length);
                offset += 4;
                break;
            default:
                assert(!"Message type not handled...");
        }
        memcpy(&frame[offset], subData, length);
        offset += length;
    }

    m2mp_client_net_send(this, frame, offset);

    // In the end, we just release the generated frame
    free(frame);
}

void m2mp_client_net_send(m2mp_client * this, BYTE * data, size_t length) {
    
    if (DO_I_LOG(LVL_DEBUG)) {
		LOG(LVL_DEBUG, "m2mp_client_net_send( ");
        m2mp_client_debug_render_bytes(data, length);
		printf(" );");
		fflush(stdout);
	}

    size_t sentBytes = send(this->socketFD, data, length, 0);

    if (length != sentBytes)
        LOG(LVL_CRITICAL, "ERROR: length=%d; sentBytes=%d", (int) length, (int) sentBytes);
}

int m2mp_client_net_receive(m2mp_client * this, int size, int timeout /* = 500 */) {
    LOG(LVL_DEBUG, "m2mp_client_net_receive( %d, %d ) : ", size, timeout);

    int rv = poll(& this->socketPollFDs, 1, timeout);

    if (rv == -1) {
        //LOG(LVL_CRITICAL, "ERROR: poll error.");

        m2mp_client_event * event = (m2mp_client_event *) malloc(sizeof ( m2mp_client_event));
        event->type = M2MP_CLIENT_EVENT_POLL_ERROR;
        m2mp_client_add_event(this, (m2mp_client_event *) event);
    } else if (rv == 0) {
        LOG(LVL_DEBUG, "Poll timeout...");
    } else {
        if (this->socketPollFDs.revents & POLLIN) {
            m2mp_client_prepare_recv_buffer(this, this->recvBufferPosition + size);


            int bytesReceived = recv(this->socketFD, & this->recvBuffer[ this->recvBufferPosition ], size, 0);

            if (bytesReceived > 0) {
                if (DO_I_LOG(LVL_DEBUG))
                    m2mp_client_debug_render_bytes(& this->recvBuffer[ this->recvBufferPosition ], bytesReceived);
                this->recvBufferPosition += bytesReceived;
            } else if (bytesReceived < 0) {
                LOG(LVL_CRITICAL, "ERROR: Socket error !");

                m2mp_client_event_disconnected * event = (m2mp_client_event_disconnected *) malloc(sizeof ( m2mp_client_event_disconnected));
                event->base.type = M2MP_CLIENT_EVENT_DISCONNECTED;
                m2mp_client_add_event(this, (m2mp_client_event *) event);
            } else if (bytesReceived == 0) {
                LOG(LVL_DEBUG, "ERROR: Socket disconnected !");

                m2mp_client_event_disconnected * event = (m2mp_client_event_disconnected *) malloc(sizeof ( m2mp_client_event_disconnected));
                event->base.type = M2MP_CLIENT_EVENT_DISCONNECTED;
                m2mp_client_add_event(this, (m2mp_client_event *) event);
            }

            return bytesReceived;
        } else if (this->socketPollFDs.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            LOG(LVL_DEBUG, "ERROR: Problem with socket...");
            return -1;
        } else if (this->socketPollFDs.revents & POLLPRI) {
            LOG(LVL_CRITICAL, "ERROR: POLLPRI event not handled");
            return -1;
        }
    }
    return rv;
}

int m2mp_client_receive(m2mp_client* this, int timeout /* = 500 */) {
    LOG(LVL_DEBUG, "m2mp_client_receive( %d );", timeout);
    // If we don't have anything yet...
    if (this->recvBufferPosition == 0) {
        // Just to give the idea:
        // m2mp_client_prepare_recv_buffer( this, 1 );
        int bytesReceived = m2mp_client_net_receive(this, 1, timeout);

        //LOG(LVL_DEBUG, "bytesReceived : %d", bytesReceived);

        if (bytesReceived <= 0)
            return bytesReceived;
    }
    //LOG(LVL_DEBUG, "recvBufferPosition = %d", (unsigned int) this->recvBufferPosition);



    // If we have the packet type
    BYTE packetType = this->recvBuffer[0];

    // If we don't know what size of headers to expect yet...
    if (!this->recvBufferExpectedHeaderSize) {
        if (packetType == PROT_R_IDENT_RESULT || packetType == PROT_R_ACK_REQUEST || packetType == PROT_R_ACK_RESPONSE) {
            this->recvBufferExpectedHeaderSize = 2;
            this->recvBufferExpectedFrameSize = 2;
        } else if (packetType <= PROT_MSG_1BYTESIZED_MAX)
            this->recvBufferExpectedHeaderSize = 2;
        else if (packetType <= PROT_MSG_2BYTESSIZED_MAX)
            this->recvBufferExpectedHeaderSize = 3;
        else if (packetType <= PROT_MSG_4BYTESSIZED_MAX)
            this->recvBufferExpectedHeaderSize = 5;
        else
            LOG(LVL_CRITICAL, "ERROR: Frame type isn't handled : %d", (unsigned int) packetType);
    }

    //LOG(LVL_DEBUG, "recvBufferExpectedHeaderSize = %d", (unsigned int) this->recvBufferExpectedHeaderSize);

    // If we know what size of headers to expect but haven't reached it yet
    if (this->recvBufferExpectedHeaderSize > this->recvBufferPosition) {
        int bytesReceived = m2mp_client_net_receive(this, this->recvBufferExpectedHeaderSize - this->recvBufferPosition, 500);

        //LOG(LVL_DEBUG, "bytesReceived : %d", bytesReceived);

        if (bytesReceived <= 0)
            return bytesReceived;
    }



    // If we still don't know the exact size of the packet but have a complete header
    if (!this->recvBufferExpectedFrameSize && this->recvBufferPosition == this->recvBufferExpectedHeaderSize) {

        if (packetType <= PROT_MSG_1BYTESIZED_MAX)
            this->recvBufferExpectedFrameSize = 2 + (size_t) this->recvBuffer[1];
        else if (packetType <= PROT_MSG_2BYTESSIZED_MAX) {
            // Length is given in little endian
#if 1 // Sometimes, le16toh doesn't exist
            uint16_t size = le16toh(* ((uint16_t *) & this->recvBuffer[ 1 ]));
#else
            uint16_t size = *((uint16_t *) & this->recvBuffer[ 1 ]);
#endif
            this->recvBufferExpectedFrameSize = 3 + (size_t) size;
        } else if (packetType <= PROT_MSG_4BYTESSIZED_MAX) {
            // Length is given in little endian
#if 1 // Sometimes, le32toh doesn't exist
            uint32_t size = le32toh(* ((uint32_t *) & this->recvBuffer[ 1 ]));
#else
            uint32_t size = *((uint32_t *) & this->recvBuffer[ 1 ]);
#endif
            this->recvBufferExpectedFrameSize = 5 + (size_t) size;
        }
    }

    //LOG(LVL_DEBUG, "recvBufferExpectedFrameSize = %d", (unsigned int) this->recvBufferExpectedFrameSize);

    // If we know the exact size of the packet but haven't reached it yet...
    if (this->recvBufferExpectedFrameSize > this->recvBufferPosition) {
        int bytesReceived = m2mp_client_net_receive(this, this->recvBufferExpectedFrameSize - this->recvBufferPosition, 500);

        //LOG(LVL_DEBUG, "bytesReceived : %d", bytesReceived);

        if (bytesReceived <= 0)
            return bytesReceived;

    }


    // If we have the right packet
    if (this->recvBufferExpectedFrameSize == this->recvBufferPosition) {
        m2mp_client_treat_frame(this, this->recvBuffer, this->recvBufferPosition);
		m2mp_client_received_packet_processed( this );
    }

    return EXIT_SUCCESS;
}

int m2mp_client_treat_frame(m2mp_client* this, BYTE* data, size_t length) {
    LOG(LVL_DEBUG, "m2mp_client_treat_frame( ");

    if (DO_I_LOG(LVL_DEBUG)) {
        m2mp_client_debug_render_bytes(data, length);
		printf(" );");
		fflush(stdout);
	}
    
    if (data[0] == PROT_R_IDENT_RESULT) {
        this->state = M2MP_CLIENT_STATE_IDENTIFIED;
        m2mp_client_event_identified * event = (m2mp_client_event_identified *) malloc(sizeof ( m2mp_client_event_identified));
        event->base.type = M2MP_CLIENT_EVENT_IDENTIFIED_RESPONSE;
        event->status = data[1];
        m2mp_client_add_event(this, (m2mp_client_event*) event);
    } else if (data[0] == PROT_R_NC_DATA) {
        m2mp_client_event_data * event = (m2mp_client_event_data *) malloc(sizeof (m2mp_client_event_data));
        event->base.type = M2MP_CLIENT_EVENT_DATA;
        event->length = (data[1] - 1);
        event->channelName = strdup(m2mp_client_channels_get_name(this->recvChannels, (int) data[2]));
        event->data = (BYTE *) malloc(sizeof (BYTE) * event->length);
        memcpy(event->data, & data[3], event->length);
        m2mp_client_add_event(this, (m2mp_client_event*) event);
    } else if (data[0] == PROT_R_NC_DATA_LARGE) {
        m2mp_client_event_data *event = (m2mp_client_event_data *) malloc(sizeof (m2mp_client_event_data));
        event->base.type = M2MP_CLIENT_EVENT_DATA;
        event->length = (size_t) le16toh(&data[1]) - 1;
        event->channelName = strdup(m2mp_client_channels_get_name(this->recvChannels, (int) data[3]));
        event->data = (BYTE *) malloc(sizeof (BYTE) * event->length);
        memcpy(event->data, & data[4], event->length);
        m2mp_client_add_event(this, (m2mp_client_event*) event);
    } else if (data[0] == PROT_R_NC_DATA_VERYLARGE) {
        m2mp_client_event_data * event = (m2mp_client_event_data *) malloc(sizeof (m2mp_client_event_data));
        event->base.type = M2MP_CLIENT_EVENT_DATA;
        event->length = (size_t) le32toh(&data[1]) - 1;
        event->channelName = strdup(m2mp_client_channels_get_name(this->recvChannels, (int) data[5]));
        event->data = (BYTE *) malloc(sizeof (BYTE) * event->length);
        memcpy(event->data, &data[6], event->length);
        m2mp_client_add_event(this, (m2mp_client_event*) event);
    } else if (
            data[0] == PROT_R_NC_DATAARRAY ||
            data[0] == PROT_R_NC_DATAARRAY_LARGE ||
            data[0] == PROT_R_NC_DATAARRAY_VERYLARGE) {
        m2mp_client_event_dataarray * event = (m2mp_client_event_dataarray *) malloc(sizeof (m2mp_client_event_dataarray));
        event->base.type = M2MP_CLIENT_EVENT_DATAARRAY;

        linkedlist listLength, listData;
        linkedlist_init(& listLength);
        linkedlist_init(& listData);


        size_t offset;
        size_t totalLength;

        if (data[0] == PROT_R_NC_DATAARRAY) {
            totalLength = (size_t) data[1] + 2;
            event->channelName = strdup(m2mp_client_channels_get_name(this->recvChannels, (int) data[2]));
            offset = 3;

            // We add each element to a linked list
            while (offset < totalLength) {
                size_t length = (size_t) data[ offset ];
                linkedlist_insert_last(&listLength, (void *) length);
                //LOG( LVL_DEBUG, "length = %d", length );
                offset += 1;

                BYTE * subData = (BYTE *) malloc(sizeof (BYTE) * length);
                memcpy(subData, & data[ offset ], length);
                linkedlist_insert_last(& listData, (void *) subData);
                offset += length;
            }
        } else if (data[0] == PROT_R_NC_DATAARRAY_LARGE) {
            totalLength = (size_t) (uint16_t) le16toh(*((uint16_t *) & data[1])) + 3;
            event->channelName = strdup(m2mp_client_channels_get_name(this->recvChannels, (int) data[3]));
            offset = 4;

            // We add each element to a linked list
            while (offset < totalLength) {
                size_t length = (size_t) (uint16_t) le16toh(*((uint16_t *) & data[ offset ]));
                linkedlist_insert_last(&listLength, (void *) length);
                //LOG( LVL_DEBUG, "length = %d", length );
                offset += sizeof (uint16_t);

                BYTE * subData = (BYTE *) malloc(sizeof (BYTE) * length);
                memcpy(subData, & data[ offset ], length);
                linkedlist_insert_last(& listData, (void *) subData);
                offset += length;
            }
        } else if (data[0] == PROT_R_NC_DATAARRAY_VERYLARGE) {
            totalLength = (size_t) (uint32_t) le32toh(*((uint32_t *) & data[1])) + 5;
            event->channelName = strdup(m2mp_client_channels_get_name(this->recvChannels, (int) data[5]));
            offset = 6;

            // We add each element to a linked list
            while (offset < totalLength) {
                size_t length = (size_t) (uint32_t) le32toh(*((uint32_t *) & data[ offset ]));
                linkedlist_insert_last(&listLength, (void *) length);
                //LOG( LVL_DEBUG, "length = %d", length );
                offset += sizeof (uint32_t);

                BYTE * subData = (BYTE *) malloc(sizeof (BYTE) * length);
                memcpy(subData, & data[ offset ], length);
                linkedlist_insert_last(& listData, (void *) subData);
                offset += length;
            }
        }

        size_t size = linkedlist_get_size(& listLength);
        size_t * lengths = (size_t *) malloc(sizeof (size_t) * size);
        BYTE ** dataArray = (BYTE **) malloc(sizeof (BYTE *) * (size + 1));

        int i;
        for (i = 0; i < size; ++i) {
            lengths[i] = (size_t) linkedlist_pop_first(& listLength);
            dataArray[i] = (BYTE *) linkedlist_pop_first(& listData);
        }
        dataArray[ size ] = (void *) NULL;

        event->lengths = lengths;
        event->data = dataArray;
        event->size = size;

        m2mp_client_add_event(this, (m2mp_client_event*) event);

        linkedlist_empty(& listLength, NULL);
        linkedlist_empty(& listData, NULL);
    } else if (data[0] == PROT_R_NC_DEF) {
        size_t len = data[1];
        char * channelName = malloc(sizeof (char) * len);
        memcpy(channelName, & data[3], len - 1);
        channelName[ (len - 1) ] = '\0';

        //LOG( LVL_DEBUG, "channelName = \"%s\"", channelName );

        m2mp_client_channels_set_id_and_name(this->recvChannels, data[2], channelName);

        free(channelName);
    } else if (data[0] == PROT_R_ACK_REQUEST) {
        m2mp_client_event_ack_request * event = (m2mp_client_event_ack_request*) malloc(sizeof ( m2mp_client_event_ack_request));
        event->base.type = M2MP_CLIENT_EVENT_ACK_REQUEST;
        event->ackNb = data[1];
        m2mp_client_add_event(this, (m2mp_client_event *) event);
    } else if (data[0] == PROT_R_ACK_RESPONSE) {
        m2mp_client_event_ack_response * event = (m2mp_client_event_ack_response*) malloc(sizeof ( m2mp_client_event_ack_response));
        event->base.type = M2MP_CLIENT_EVENT_ACK_RESPONSE;
        event->ackNb = data[1];
        m2mp_client_add_event(this, (m2mp_client_event *) event);
    } else {
        LOG(LVL_DEBUG, "ERROR: Packet not handled... data[0] = 0x%02X", (unsigned int) data[0]);
    }

    return EXIT_SUCCESS;
}

int m2mp_client_connection_check(m2mp_client * this) {
    if (((time(NULL) - this->lastRecvTime) > 15 * 60) && (time(NULL) - this->lastAckSent) > 30) {
        m2mp_client_send_ack_request(this);
    }

    return EXIT_SUCCESS;
}

m2mp_client_event * m2mp_client_work(m2mp_client * this, int timeout /* = 500 */) {
    // If we don't have any event waiting to be sent, we can work...
    if (!linkedlist_get_size(& this->events))
        m2mp_client_receive(this, timeout);

    m2mp_client_event * event = (m2mp_client_event *) linkedlist_pop_first(& this->events);

    if (event) {
        if (event->type == M2MP_CLIENT_EVENT_DISCONNECTED) {
            m2mp_client_init_disconnected(this);
        }
		else if ( event->type == M2MP_CLIENT_EVENT_DATAARRAY ) {
			m2mp_client_event_dataarray * eventDa = (m2mp_client_event_dataarray*) event;
			
			// If it's on the status channel
			if ( !strcmp(eventDa->channelName, "_sta") ) {
				// We get the firt cell, the command
				char * cmd = m2mp_client_event_dataarray_get_string( eventDa, 0 );
				
				// For the "GET" command ...
				if ( ! strcmp(cmd,"g")) {
					char * name = m2mp_client_event_dataarray_get_string( eventDa, 1 );
					char * value = dictionnary_get_value( & this->statuses, name );

					// If we found something
					if ( value ) {
						// We need to create a string in the form <name>=<value>
						char * returned = strdup(name);
						str_append(&returned, "=");
						str_append(&returned,value);
						const char *tab[] =  { "g", returned, NULL };
						m2mp_client_send_string_array( this, "_sta", tab );
						free( returned );
					}
					else {
						const char * tab[] = { "g", NULL };
						m2mp_client_send_string_array( this, "_sta", tab );
					}
				}
			}
			
		}
		else if (event->type == M2MP_CLIENT_EVENT_DATA) {
            //m2mp_client_event_data * dataEvent = (m2mp_client_event_data*) event;
			
			// This is NOT supported anymore
/*
            if (!strcmp(dataEvent->channelName, "_cap") && dataEvent->length == 1 && dataEvent->data[0] == '?') {
                m2mp_client_send_string_array(this, "_cap", (const char **) this->capabilities);
            }
*/
        } else if (event->type == M2MP_CLIENT_EVENT_ACK_REQUEST && this->autoReplyToAckRequests) {
            m2mp_client_event_ack_request * ackRequest = (m2mp_client_event_ack_request *) event;
            m2mp_client_send_ack_response(this, ackRequest->ackNb);
        }
    }

    int i = 0;
    m2mp_client_plugin_entry * entry;
    while ((entry = this->pluginsArray[i++ ])) {
        m2mp_client_event ** events = entry->func(this, entry->pluginInstance, event);
		
		// If method does not return a null pointer
		if ( events ) {
			// Then we queue every event inside
			int i = 0;
			while( events[i] ) {
				m2mp_client_add_event(this, events[i]);
				i++;
			}
			// We free the current array
			free( events );
			
			// And the next event becomes the next event
			event = m2mp_client_get_event(this);
			
			// NOTE: This means that to totally remove an event, the called plugin
			// can just returned an empty array. To add some events, it just has to
			// to return the event plus some other ones.
		}
	}

    return event;
}

void m2mp_client_debug_render_bytes(BYTE * data, size_t length) {
    printf("BYTE[ %d ] {", (int) length);

    int i;
    for (i = 0; i < length; ++i)
        printf(" 0x%02X", data[i]);

    printf(" }");
}

void m2mp_client_prepare_send_buffer(m2mp_client* this, size_t size) {
    if (this->sendBufferSize < size){
    if (this->sendBuffer)
        this->sendBuffer = realloc(this->sendBuffer, size);
    else
        this->sendBuffer = malloc(size);
	}
}

void m2mp_client_prepare_recv_buffer(m2mp_client* this, size_t size) {
    if (this->recvBufferSize < size) {
        if (this->recvBuffer)
            this->recvBuffer = realloc(this->recvBuffer, size);
        else
            this->recvBuffer = malloc(size);
    }
}

void m2mp_client_send_string(m2mp_client * this, const char * channel_name, const char * string) {
    LOG(LVL_VERBOSE, "m2mp_client_send_string(  \"%s\", \"%s\" );", channel_name, string);
    m2mp_client_send_data(this, channel_name, (BYTE *) string, strlen(string));
}

void m2mp_client_send_string_array(m2mp_client* this, const char* channel_name, const char** strings) {
    LOG(LVL_VERBOSE, "m2mp_client_send_string_array( \"%s\", [...] );", channel_name);

    size_t strings_count;
    for (strings_count = 0; strings[strings_count]; ++strings_count)
        ;

    LOG(LVL_VERBOSE, "strings_count=%d", (int) strings_count);
    size_t * lengths = (size_t *) malloc(sizeof (size_t) * strings_count);

    int i;
    for (i = 0; strings[i]; ++i)
        lengths[i] = strlen(strings[i]);

    m2mp_client_send_data_array(this, channel_name, (const BYTE **) strings, lengths);

    free(lengths);
}

char * m2mp_client_channels_get_name(char * channels[256], const unsigned char id) {
    assert(id >= 0 && id <= 255);

	assert( channels[id] );
	
    return channels[id];
}

void m2mp_client_channels_set_id_and_name(char * channels[256], const unsigned char id, const char * name) {
    //channels[id] = name;
    LOG(LVL_DEBUG, "m2mp_client_channels_set_id_and_name( [...], %d, \"%s\" );", id, name);

    if (channels[id])
        free(channels[id]);

    channels[id] = strdup(name);
}

int m2mp_client_channels_get_id(m2mp_client * this, char * channels[256], const char * name) {
    assert(name);

    int firstEmpty = -1;

    // We search for a channel a detect the first empty channel
    int i;
    for (i = 0; i < 256; ++i) {
        if (!channels[i]) {
            if (firstEmpty == -1)
                firstEmpty = i;
            continue;
        }
        if (!strcmp(channels[i], name))
            return i;
    }

    // If we couldn't find an empty channel, we release all the channels
    if (firstEmpty == -1) {
        m2mp_client_channels_free(channels);
        firstEmpty = 0;
    }

    // We save the channel but we copy it first
	char * name_copy = strdup(name);
    channels[firstEmpty] = name_copy;

    m2mp_client_send_channel_id_and_name(this, (unsigned char) firstEmpty, name_copy);

    return firstEmpty;
}

void m2mp_client_add_event(m2mp_client* this, m2mp_client_event* event) {
    linkedlist_insert_last(& this->events, event);
}

m2mp_client_event * m2mp_client_get_event(m2mp_client* this) {
    return (m2mp_client_event *) linkedlist_pop_first(& this->events);
}

void m2mp_client_channels_init(char * channels[256]) {
    int i;

    for (i = 0; i < 256; ++i)
        channels[i] = NULL;
}

void m2mp_client_channels_free(char * channels[256]) {
    int i;

    for (i = 0; i < 256; ++i) {
        if (channels[i]) {
            free(channels[i]);
            channels[i] = NULL;
        }
    }
}

char * m2mp_client_event_data_get_string(m2mp_client_event_data * this) {
    char * str = (char *) malloc(sizeof (char) * (this->length + 1));

    memcpy(str, this->data, this->length);

    str[ this->length ] = '\0';

    return str;
}

char * m2mp_client_event_dataarray_get_string(m2mp_client_event_dataarray * this, int i) {
    size_t length = this->lengths[i];

    char * str = (char *) malloc(sizeof (char) * (length + 1));

    memcpy(str, this->data[i], length);

    str[ length ] = '\0';

    return str;
}

void m2mp_client_add_plugin(m2mp_client * this, void * instance, m2mp_client_event ** (*func)(void * clientInstance, void * pluginInstance, m2mp_client_event * event)) {
    m2mp_client_plugin_entry * entry = (m2mp_client_plugin_entry *) malloc(sizeof (m2mp_client_plugin_entry));
    entry->pluginInstance = instance;
    entry->func = func;

    linkedlist_insert_last(& this->pluginsLinkedList, entry);

    m2mp_client_update_plugin_array(this);
}

void m2mp_client_rmv_plugin(m2mp_client* this, void* instance) {
    linkedlist_node * node = linkedlist_get_first_node(& this->pluginsLinkedList);

    while (node) {

        m2mp_client_plugin_entry * entry = node->element;

        // We search for the plugin
        if (entry->pluginInstance == instance) {

            // Then we remove the plugin
            linkedlist_remove(& this->pluginsLinkedList, entry);

            // And we delete the m2mp_client_plugin_entry
            free(entry);
            return;
        }

        node = node->next;
    }

    m2mp_client_update_plugin_array(this);
}

void m2mp_client_update_plugin_array(m2mp_client* this) {
    if (this->pluginsArray)
        free(this->pluginsArray);

    this->pluginsArray = (m2mp_client_plugin_entry **) linkedlist_get_array(& this->pluginsLinkedList);
    //mw_add(this->pluginsArray, __FILE__, __LINE__, __FUNCTION__);
}

int m2mp_client_register_event_type(m2mp_client* this) {
	int type = this->event_types;
	int offset =  type - M2MP_CLIENT_EVENT_PLUGINS;
	this->event_types++;
	this->event_destructors = (m2mp_client_event_destructor*) realloc( this->event_destructors, sizeof( m2mp_client_event_destructor ) * (offset+1) );
	memset( & this->event_destructors[ offset], 0, sizeof( m2mp_client_event_destructor ) );
	return type;
}

void m2mp_client_register_event_destructor(m2mp_client* this, int event_id, void (*func)(m2mp_client_event*)) {
	int offset = event_id - M2MP_CLIENT_EVENT_PLUGINS;
	this->event_destructors[ offset].eventId = event_id;
	this->event_destructors[ offset].destructor = func;
}

