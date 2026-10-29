#pragma once
#ifndef _CHESS_BOARD_H
#define _CHESS_BOARD_H

#include "chess_piece.h"
#include <stdbool.h>

typedef struct ChessBoard {
	struct ChessPiece pieces[8][8];
	struct ChessPiece previousState[8][8];
} ChessBoard;

typedef struct PiecePosition {
	int x;
	int y;
} PiecePosition;

extern const ChessBoard standardBoard;

enum MoveResult {
	// invalid
	MOVE_INVALID = 0,
	MOVE_INVALID_OUT_OF_RANGE,
	MOVE_INVALID_OUT_OF_PIECE_RANGE,
	MOVE_INVALID_SAME_COLOR,
	MOVE_INVALID_CANNOT_CASTLE,
	MOVE_INVALID_CHECK,
	MOVE_INVALID_HAS_MOVED,
	MOVE_INVALID_HAS_OBSTACLE,

	// valid
	MOVE_VALID,
	MOVE_VALID_MOVE,
	MOVE_VALID_CAPTURE,
	MOVE_VALID_CASTLE,
	MOVE_VALID_CHECK,

	MOVE_COUNT
};

// general
void print_board(const ChessBoard board);
bool checkRange(const ChessBoard board, PiecePosition pos);
bool checkMoveRange(const ChessBoard board, PiecePosition start, PiecePosition end);
bool isKingChecked(const ChessBoard board, enum PIECE_COLOR color);
bool isMoveSafe(const ChessBoard board, PiecePosition start, PiecePosition end);
bool isCaptureSafe(const ChessBoard board, PiecePosition start, PiecePosition end);
bool isPiecePresent(const ChessBoard board, PiecePosition position);
enum MoveResult movePiece(ChessBoard* board, PiecePosition start, PiecePosition end);
ChessPiece getPieceAt(const ChessBoard board, PiecePosition position);

char* PieceToString(enum PIECE_TYPE type);

int GetAvailableMoves(const struct ChessBoard board, struct PiecePosition positionsBuffer[64], struct PiecePosition pos);

// check each piece
// Pawn
enum MoveResult isValidPawnMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing);
// Rook
enum MoveResult isValidRookMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing);
// Knight
enum MoveResult isValidKnightMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing);
// Bishop
enum MoveResult isValidBishopMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing);
// King
enum MoveResult isValidKingMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing);
// Queen
enum MoveResult isValidQueenMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing);

#endif // _CHESS_BOARD_H
