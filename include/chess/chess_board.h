#pragma once
#ifndef _CHESS_BOARD_H
#define _CHESS_BOARD_H

#include "chess_piece.h"
#include <unordered_map>
#include <string>

struct ChessBoard {
	ChessPiece pieces[8][8];
	ChessPiece previousState[8][8];
};

struct PiecePosition {
	int x;
	int y;
};

const ChessBoard standardBoard = {
	.pieces = {
		{ {.color = COLOR_BLACK, .type = PIECE_ROOK }, {.color = COLOR_BLACK, .type = PIECE_KNIGHT }, {.color = COLOR_BLACK, .type = PIECE_BISHOP }, {.color = COLOR_BLACK, .type = PIECE_QUEEN }, {.color = COLOR_BLACK, .type = PIECE_KING }, {.color = COLOR_BLACK, .type = PIECE_BISHOP }, {.color = COLOR_BLACK, .type = PIECE_KNIGHT }, {.color = COLOR_BLACK, .type = PIECE_ROOK } },
		{ {.color = COLOR_BLACK, .type = PIECE_PAWN }, {.color = COLOR_BLACK, .type = PIECE_PAWN }, {.color = COLOR_BLACK, .type = PIECE_PAWN }, {.color = COLOR_BLACK, .type = PIECE_PAWN }, {.color = COLOR_BLACK, .type = PIECE_PAWN }, {.color = COLOR_BLACK, .type = PIECE_PAWN }, {.color = COLOR_BLACK, .type = PIECE_PAWN }, {.color = COLOR_BLACK, .type = PIECE_PAWN } },
		{ {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE } },
		{ {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE } },
		{ {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE } },
		{ {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE }, {.color = COLOR_NONE, .type = PIECE_NONE } },
		{ {.color = COLOR_WHITE, .type = PIECE_PAWN }, {.color = COLOR_WHITE, .type = PIECE_PAWN }, {.color = COLOR_WHITE, .type = PIECE_PAWN }, {.color = COLOR_WHITE, .type = PIECE_PAWN }, {.color = COLOR_WHITE, .type = PIECE_PAWN }, {.color = COLOR_WHITE, .type = PIECE_PAWN }, {.color = COLOR_WHITE, .type = PIECE_PAWN }, {.color = COLOR_WHITE, .type = PIECE_PAWN } },
		{ {.color = COLOR_WHITE, .type = PIECE_ROOK }, {.color = COLOR_WHITE, .type = PIECE_KNIGHT }, {.color = COLOR_WHITE, .type = PIECE_BISHOP }, {.color = COLOR_WHITE, .type = PIECE_QUEEN }, {.color = COLOR_WHITE, .type = PIECE_KING }, {.color = COLOR_WHITE, .type = PIECE_BISHOP }, {.color = COLOR_WHITE, .type = PIECE_KNIGHT }, {.color = COLOR_WHITE, .type = PIECE_ROOK} }
	}
};


enum MoveResult {
	// invalid
	MOVE_INVALID = 0,
	MOVE_INVALID_OUT_OF_RANGE,
	MOVE_INVALID_OUT_OF_PIECE_RANGE,
	MOVE_INVALID_SAME_COLOR,
	MOVE_INVALID_CHECK,
	MOVE_INVALID_HAS_MOVED,
	MOVE_INVALID_HAS_OBSTACLE,

	// valid
	MOVE_VALID,
	MOVE_VALID_MOVE,
	MOVE_VALID_CAPTURE,
	MOVE_VALID_CHECK,

	MOVE_COUNT
};

// general
void print_board(const ChessBoard board);
bool checkRange(const ChessBoard board, PiecePosition pos);
bool checkMoveRange(const ChessBoard board, PiecePosition start, PiecePosition end);
bool isKingChecked(const ChessBoard board, PIECE_COLOR color);
bool isMoveSafe(const ChessBoard& board, PiecePosition start, PiecePosition end);
bool isCaptureSafe(const ChessBoard& board, PiecePosition start, PiecePosition end);
MoveResult movePiece(ChessBoard& board, PiecePosition start, PiecePosition end);
ChessPiece getPieceAt(const ChessBoard& board, PiecePosition position);
#include <string>

std::string PieceToString(PIECE_TYPE type);

// check each piece
// Pawn
MoveResult isValidPawnMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing=false);
// Rook
MoveResult isValidRookMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing=false);
// Knight
MoveResult isValidKnightMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing=false);
// Bishop
MoveResult isValidBishopMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing=false);
// King
MoveResult isValidKingMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing=false);
// Queen
MoveResult isValidQueenMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing=false);

#endif // _CHESS_BOARD_H
