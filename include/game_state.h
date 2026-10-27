#pragma once
#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

// chess implementation
#include "chess/chess_board.h"
#include <sys/types.h>

#define MAX_PLAYERS 2

typedef struct GameState {
	// handle player IDs
	// reserve 0 for hehe po :3
	int playerIDtoFD[MAX_PLAYERS + 1];
	enum PIECE_COLOR playerIDtoColor[MAX_PLAYERS + 1];
	size_t currentPlayerCount;

	int currentPlayerIDTurn;
	enum PIECE_COLOR currentColorTurn;

	bool gameStarted;

	// chessBoard
	struct ChessBoard currentChessBoard;
	bool isRotated;
	enum PIECE_COLOR checkedColor;
} GameState;

#ifdef __cplusplus
}
#endif

#endif // _GAME_STATE_H
