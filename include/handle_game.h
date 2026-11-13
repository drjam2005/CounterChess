#pragma once
#ifndef HANDLE_GAME_H
#define HANDLE_GAME_H

#include "../include/game_state.h"
#include "../include/packet_handler.h"
#include "../include/my_structs.h"

typedef struct GameInfo {
	bool broadcastGameState;
} GameInfo;

bool handleGameState(GameState*, fromClientPacketQueue*, struct GameInfo, queueServerStruct stru);

#endif
