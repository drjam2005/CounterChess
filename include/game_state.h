#pragma once
#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#include "packets.h"
#include <deque>

struct GameState {
	int player1FD = -1;
	int player2FD = -1;

	int currentPlayerTurn = -1;
};

bool handleGameState(GameState& gameState, std::deque<fromClientPacket>& currentPacketQueue);

#endif
