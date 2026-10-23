#pragma once
#ifndef _RENDER_ASSETS_H
#define _RENDER_ASSETS_H

#include "../vendor/raylib/include/raylib.h"
#include "chess/chess_piece.h"
#include "chess/chess_board.h"
#include "game_state.h"

struct RenderInfo {
	PiecePosition chosenStart;
	PIECE_COLOR playerColor;
};

void renderBoard(const GameState& gameState, RenderInfo info);

#endif // _RENDER_ASSETS_H
