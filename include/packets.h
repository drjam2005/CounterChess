#pragma once
#ifndef _PACKET_H
#define _PACKET_H

#include <string>
#include "game_state.h"

enum PACKET_TYPE {
	// from server / to client
	PACKET_NULL = 0,
	PACKET_SEND_ID,
	PACKET_SERVER_FULL,
	PACKET_SEND_GAME_STATE,
	PACKET_SERVER_SHUTDOWN,
	PACKET_INFORM_PLAYER_NOT_TURN,
	PACKET_INFORM_PLAYER_TURN,

	// from client / to server
	PACKET_SEND_MOVE,
	PACKET_SEND_MESSAGE,

	// misc
	PACKET_COUNT
};

struct fromServerPacket {
	// arbitrary data
	PACKET_TYPE packetType;
	int receivingFD;
	int providedPlayerID;
	GameState gameState;

	// for messages
	int messageFromID;
	char message[256];
};

struct fromClientPacket {
	// arbitrary data
	PACKET_TYPE packetType;
	// only server thing
	int clientID;
	int clientFD;

	int chosenMove;

	char message[256];
};

#define toClientPacket fromServerPacket
#define toServerPacket fromClientPacket

#endif // _PACKET_H
