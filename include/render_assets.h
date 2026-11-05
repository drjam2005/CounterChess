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
struct PiecePosition toBoardPosition(int x, int y, enum PIECE_COLOR playerColor);

#endif // _RENDER_ASSETS_H
