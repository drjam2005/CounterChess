#pragma once
#ifndef _PACKET_H
#define _PACKET_H

#include <string>

enum PACKET_TYPE {
	// to client
	PACKET_NULL = 0,
	PACKET_SEND_ID,
	PACKET_SERVER_FULL,
	PACKET_SEND_STATE,

	// from client
	PACKET_SEND_MOVE,

};

struct ClientMessage {
	int clientFD;
	int clientID;
	char message[256];
};

struct fromServerPacket {
	// arbitrary data
	PACKET_TYPE packetType;
	int ID;
	char message[256];
};

struct fromClientPacket {
	// arbitrary data
	PACKET_TYPE packetType;
	// only server thing
	int clientFD;

	int chosenMove;
};

#endif // _PACKET_H
