#pragma once
#ifndef _CHESS_BOARD_H
#define _CHESS_BOARD_H

#include "chess_piece.h"
#include <unordered_map>

struct ChessBoard {
	ChessPiece pieces[8][8];
	//
	ChessPiece previousState[8][8];
};

struct PiecePosition {
	size_t x;
	size_t y;
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
	// valid
	MOVE_VALID = 0,
	MOVE_VALID_CAPTURE,
	MOVE_VALID_CHECK,

	// invalid
	MOVE_INVALID,
	MOVE_INVALID_OUT_OF_RANGE,
	MOVE_INVALID_SAME_COLOR,
	MOVE_INVALID_CHECK,
	MOVE_INVALID_HAS_MOVED,
	MOVE_INVALID_HAS_OBSTACLE,

	MOVE_COUNT
};


// general
void print_board(const ChessBoard board);
bool checkRange(const ChessBoard board, PiecePosition pos);
bool checkMoveRange(const ChessBoard board, PiecePosition start, PiecePosition end);
bool movePiece(ChessBoard& board, PiecePosition start, PiecePosition end);
ChessPiece getPieceAt(const ChessBoard& board, PiecePosition position);

// check each piece
// Pawn
MoveResult isValidPawnMove(const ChessBoard board, PiecePosition start, PiecePosition end);
// Rook
MoveResult isValidRookMove(const ChessBoard board, PiecePosition start, PiecePosition end);
// Knight
MoveResult isValidKnightMove(const ChessBoard board, PiecePosition start, PiecePosition end);
// Bishop
MoveResult isValidBishopMove(const ChessBoard board, PiecePosition start, PiecePosition end);
// King
MoveResult isValidKingMove(const ChessBoard board, PiecePosition start, PiecePosition end);
// Queen
MoveResult isValidQueenMove(const ChessBoard board, PiecePosition start, PiecePosition end);

#endif // _CHESS_BOARD_H
