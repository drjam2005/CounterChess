#pragma once
#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#include <deque>

// chess implementation
#include "chess/chess_board.h"

#define MAX_PLAYERS 2

struct GameState {
	// handle player IDs
	// reserve 0 for hehe po :3
	int playerIDtoFD[MAX_PLAYERS + 1];
	size_t currentPlayerCount = 0;

	int currentPlayerIDTurn = -1;

	bool gameStarted = false;

	// chessBoard
	ChessBoard currentChessBoard;
	bool isRotated = false;

};


#endif
