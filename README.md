# M2MP C client library

[![Build Status](https://drone.io/github.com/fclairamb/m2mp-client-c/status.png)](https://drone.io/github.com/fclairamb/m2mp-client-c/latest)

## Introduction 
This is a simple C implementation of M2MP protocol.

## Current status of the project
The project isn't intended to be used by anyone yet. In the future, I'd like it to be integrated in simple embedded hardware.

## How to compile
This should get you started:

    git clone git://github.com/fclairamb/m2mp-client-c.git
    cd m2mp-client-c
    make run

## The M2MP protocol
Before using this library, you can read the specifications of the [M2MP Protocol](http://florent.clairambault.fr/m2mp-protocol). 

Short descriptions:
* Multiplexing protocol (not pub/sub)
* Consumed services declared by its defined capabilities (asked at the first connection, can be redefined lated)
* Supports two ways of sending data :
 * Byte array on a named channel (see m2mp_client_send_data and m2mp_client_send_string )
 * Array of byte arrays on a named channel (see m2mp_client_send_data_array and m2mp_client_send_string_array )
* Some services and their associated channels are pre-defined:
 * Settings management ("_set" channel) to manage settings: All the parameters that can be changed on device or on server
 * Status data management ("_sta") to manage status: All the parameters that only reflect the state of the device (can only be requested by server or sent by device)
 * Command management ("_cmd") to manage command: Free to use. It only defines a basic mechanism to receive, parse and acknowledge commands.

## The missing link
I'd like to create and offer an open-source M2MP platform associated with this library.
