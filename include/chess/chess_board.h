#pragma once
#ifndef _CHESS_BOARD_H
#define _CHESS_BOARD_H

#include "chess_piece.h"
#include <unordered_map>

struct ChessBoard {
	ChessPiece pieces[8][8];
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

// general
void print_board(const ChessBoard board);
bool checkRange(const ChessBoard board, PiecePosition start, PiecePosition end);
bool movePiece(ChessBoard& board, PiecePosition start, PiecePosition end);

// check each piece
// Pawn
bool isValidPawnMove(const ChessBoard board, PiecePosition start, PiecePosition end);
// Rook
bool isValidRookMove(const ChessBoard board, PiecePosition start, PiecePosition end);
// Knight
bool isValidKnightMove(const ChessBoard board, PiecePosition start, PiecePosition end);
// Bishop
bool isValidBishopMove(const ChessBoard board, PiecePosition start, PiecePosition end);
// King
bool isValidKingMove(const ChessBoard board, PiecePosition start, PiecePosition end);
// Queen
bool isValidQueenMove(const ChessBoard board, PiecePosition start, PiecePosition end);

#endif // _CHESS_BOARD_H
