#pragma once
#ifndef _CHESS_PIECE_H
#define _CHESS_PIECE_H

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


struct ChessPiece {
	PIECE_COLOR color;
	PIECE_TYPE type;

	PIECE_TYPE previousType;
	bool hasMoved = false;
	bool pawn2Stepped = false;
};


#endif // _CHESS_PIECE_H
