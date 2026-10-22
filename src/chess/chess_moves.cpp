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
					return MOVE_INVALID_HAS_MOVED;

				if(board.pieces[start.y+1][start.x].type != PIECE_NONE)
					return MOVE_INVALID_HAS_OBSTACLE;

				return MOVE_VALID;
			} if(end.y == start.y + 1) {
				if(board.pieces[start.y+1][start.x].type != PIECE_NONE)
					return MOVE_INVALID_HAS_OBSTACLE;

				return MOVE_VALID;
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

	if (end.x > start.x) {
		// target to the right
		for(size_t x = start.x; x != end.x-1; ++x) {
			if(getPieceAt(board, {x, start.y}).type != PIECE_NONE)
				return MOVE_INVALID_HAS_OBSTACLE;
		}
	} else if (end.x < start.x) {
		// target to the left
		for(size_t x = start.x; x != end.x+1; --x) {
			if(getPieceAt(board, {x, start.y}).type != PIECE_NONE)
				return MOVE_INVALID_HAS_OBSTACLE;
		}
	}

	if (end.y > start.y) {
		// target to the top
		for(size_t y = start.y; y != end.y-1; ++y) {
			if(getPieceAt(board, {y, start.y}).type != PIECE_NONE)
				return MOVE_INVALID_HAS_OBSTACLE;
		}
	} else if (end.y < start.y) {
		// target to the bottom
		for(size_t y = start.y; y != end.y+1; --y) {
			if(getPieceAt(board, {y, start.y}).type != PIECE_NONE)
				return MOVE_INVALID_HAS_OBSTACLE;
		}
	}

	if(rook.color == target.color)
		return MOVE_INVALID_SAME_COLOR;

	if(rook.color != target.color)
		return MOVE_VALID_CAPTURE;

	return MOVE_VALID;
}

 // Knight
MoveResult isValidKnightMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
    if (!checkMoveRange(board, start, end))
        return MOVE_INVALID_OUT_OF_RANGE;

    ChessPiece knight = board.pieces[start.y][start.x];
    ChessPiece target = board.pieces[end.y][end.x];

    int dx = fabs(end.x - start.x);
    int dy = fabs(end.y - start.y);

    // Knight must move in an L-shape
    if (!((dx == 2 && dy == 1) ||
          (dx == 1 && dy == 2)))
        return MOVE_INVALID_OUT_OF_RANGE;

    // Cannot capture a piece of the same color
    if (target.type != PIECE_NONE && knight.color == target.color)
        return MOVE_INVALID_SAME_COLOR;

	if (knight.color != target.color)
		return MOVE_VALID_CAPTURE;

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
