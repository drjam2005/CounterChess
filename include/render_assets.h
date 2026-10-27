#pragma once
#ifndef _RENDER_ASSETS_H
#define _RENDER_ASSETS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../vendor/raylib/include/raylib.h"
#include "chess/chess_piece.h"
#include "chess/chess_board.h"
#include "game_state.h"

typedef struct RenderInfo {
	struct PiecePosition chosenStart;
	enum PIECE_COLOR playerColor;
} RenderInfo;

void renderBoard(const struct GameState gameState, struct RenderInfo info);

#ifdef __cplusplus
}
#endif

#endif // _RENDER_ASSETS_H
