#pragma once
#ifndef _CHESS_PIECE_H
#define _CHESS_PIECE_H

#include <stdbool.h>

enum PIECE_COLOR {
	COLOR_NONE,

	COLOR_WHITE,
	COLOR_BLACK,

	COLOR_COUNT
};

enum PIECE_TYPE {
	PIECE_NONE,
	// for error handling
	PIECE_INVALID,

	PIECE_PAWN,
	PIECE_KNIGHT,
	PIECE_BISHOP,
	PIECE_ROOK,
	PIECE_KING,
	PIECE_QUEEN,

	PIECE_COUNT
};


typedef struct ChessPiece {
	enum PIECE_COLOR color;
	enum PIECE_TYPE type;

	enum PIECE_TYPE previousType;
	bool hasMoved;
	bool pawn2Stepped;
} ChessPiece;


#endif // _CHESS_PIECE_H
