#pragma once
#ifndef _RENDER_ASSETS_H
#define _RENDER_ASSETS_H

#include "../vendor/raylib/include/raylib.h"
#include "chess/chess_piece.h"
#include "chess/chess_board.h"
#include "game_state.h"

typedef struct RenderInfo {
	struct PiecePosition chosenStart;
	enum PIECE_COLOR playerColor;
	bool isPromoting;
} RenderInfo;

void renderBoard(const struct GameState gameState, struct RenderInfo info);
void loadPieceTextures(void);
void unloadPieceTextures(void);
struct PiecePosition toBoardPosition(int x, int y, enum PIECE_COLOR playerColor);

typedef struct RenderMessage {
	int id;
	char message[256];
} RenderMessage;

void renderMessages(RenderMessage* messages, int currentIndex, int clientID, int storedMessageCount); 

#endif // _RENDER_ASSETS_H
