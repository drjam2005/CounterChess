#include "../../include/chess/chess_board.h"
#include <math.h>

bool checkRange(const ChessBoard board, PiecePosition pos) {
	if(pos.x < 0 || pos.x >= 8)
		return false;
	if(pos.y < 0 || pos.y >= 8)
		return false;

	return true;
}

bool checkMoveRange(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if (start.x < 0 || end.x < 0 || start.x >= 8 || end.x >= 8)
		return false;
	if (start.y < 0 || end.y < 0 || start.y >= 8 || end.y >= 8)
		return false;

	return true;
}

ChessPiece getPieceAt(const ChessBoard& board, PiecePosition pos) {
	if(!checkRange(board, pos))
		return { .type = PIECE_INVALID };

	return board.pieces[pos.y][pos.x];
}

// check each piece
// Pawn
MoveResult isValidPawnMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkMoveRange(board, start, end))
		return MOVE_INVALID_OUT_OF_RANGE;

	ChessPiece pawn = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];

	if(pawn.color == COLOR_WHITE) {
		if(start.x == end.x) {
			if(end.y == start.y - 2) {
				if(pawn.hasMoved)
					return MOVE_INVALID_HAS_MOVED;

				if(board.pieces[start.y-1][start.x].type != PIECE_NONE)
					return MOVE_INVALID_HAS_OBSTACLE;

				return MOVE_VALID;
			} if(end.y == start.y - 1) {
				if(board.pieces[start.y-1][start.x].type != PIECE_NONE)
					return MOVE_INVALID_HAS_OBSTACLE;

				return MOVE_VALID;
			}
		}
		//
	} else if (pawn.color == COLOR_BLACK) {
		if(start.x == end.x) {
			if(end.y == start.y + 2) {
				if(pawn.hasMoved)
					return false;

				if(board.pieces[start.y+1][start.x].type != PIECE_NONE)
					return false;

				return true;
			} if(end.y == start.y + 1) {
				if(board.pieces[start.y+1][start.x].type != PIECE_NONE)
					return false;

				return true;
			}
		}
		//
	}

	if(pawn.color == target.color) 
		return MOVE_INVALID_SAME_COLOR;

	return MOVE_VALID;
}

// Rook
MoveResult isValidRookMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkMoveRange(board, start, end))
		return MOVE_INVALID_OUT_OF_RANGE;

	ChessPiece rook = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];

	if(end.x != start.x || end.y != start.y)
		return MOVE_INVALID_OUT_OF_RANGE;

	return MOVE_VALID;
}

// Knight
MoveResult isValidKnightMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkMoveRange(board, start, end))
		return MOVE_INVALID_OUT_OF_RANGE;

	ChessPiece knight = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	return MOVE_VALID;
}

// Bishop
MoveResult isValidBishopMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkMoveRange(board, start, end))
		return MOVE_INVALID_OUT_OF_RANGE;

	ChessPiece bishop = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	return MOVE_VALID;
}

// King
MoveResult isValidKingMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkMoveRange(board, start, end))
		return MOVE_INVALID_OUT_OF_RANGE;

	ChessPiece king = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	return MOVE_VALID;
}

// Queen
MoveResult isValidQueenMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkMoveRange(board, start, end))
		return MOVE_INVALID_OUT_OF_RANGE;

	ChessPiece queen = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	return MOVE_VALID;
}
