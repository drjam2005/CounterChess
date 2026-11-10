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

typedef enum MoveResult {
	// invalid
	MOVE_INVALID					= (1 << 0),
	MOVE_INVALID_OUT_OF_RANGE		= (1 << 1),
	MOVE_INVALID_OUT_OF_PIECE_RANGE = (1 << 2),
	MOVE_INVALID_SAME_COLOR			= (1 << 3),
	MOVE_INVALID_CANNOT_CASTLE		= (1 << 4),
	MOVE_INVALID_CHECK				= (1 << 5),
	MOVE_INVALID_HAS_MOVED			= (1 << 6),
	MOVE_INVALID_HAS_OBSTACLE		= (1 << 7),

	// valid
	MOVE_VALID						= (1 << 8),
	MOVE_VALID_MOVE					= (1 << 9),
	MOVE_VALID_CAPTURE				= (1 << 10),
	MOVE_VALID_CASTLE				= (1 << 11),
	MOVE_VALID_CHECK				= (1 << 12),
	MOVE_PROMOTION					= (1 << 13),

	MOVE_COUNT						= (1 << 14)
} MoveResult;

// general
void print_board(const ChessBoard board);
bool checkRange(const ChessBoard board, PiecePosition pos);
bool checkMoveRange(const ChessBoard board, PiecePosition start, PiecePosition end);

bool isMoveSafe(const ChessBoard board, PiecePosition start, PiecePosition end);
bool isCaptureSafe(const ChessBoard board, PiecePosition start, PiecePosition end);
bool isPiecePresent(const ChessBoard board, PiecePosition position);

bool isKingChecked(const ChessBoard board, enum PIECE_COLOR color);
bool ifCheckmate(const ChessBoard board, enum PIECE_COLOR color);
bool ifStalemate(const ChessBoard board, enum PIECE_COLOR color);
enum MoveResult evaluateMove(ChessBoard* board, PiecePosition start, PiecePosition end);
enum MoveResult movePiece(ChessBoard* board, PiecePosition start, PiecePosition end, enum PIECE_TYPE providedType);

ChessPiece getPieceAt(const ChessBoard board, PiecePosition position);

char* TypeToString(enum PIECE_TYPE type);
char* PieceToString(ChessPiece piece);

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
