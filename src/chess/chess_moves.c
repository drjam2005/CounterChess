#include "../../include/chess/chess_board.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

ChessPiece getPieceAt(const ChessBoard board, PiecePosition pos) {
	if(!checkRange(board, pos))
		return (ChessPiece){ .type = PIECE_INVALID };

	return board.pieces[pos.y][pos.x];
}

bool isPiecePresent(const ChessBoard board, PiecePosition position) {
	return getPieceAt(board, position).type != PIECE_NONE;
}


bool isMoveSafe(const ChessBoard board, PiecePosition start, PiecePosition end) {
    if (!checkMoveRange(board, start, end))
        return false;

    ChessBoard tempBoard = board;

    ChessPiece movingPiece = tempBoard.pieces[start.y][start.x];

    tempBoard.pieces[start.y][start.x].type = PIECE_NONE;
    tempBoard.pieces[end.y][end.x] = movingPiece;

    return !isKingChecked(tempBoard, movingPiece.color);
}

bool isCaptureSafe(const ChessBoard board, PiecePosition start, PiecePosition end) {
    return isMoveSafe(board, start, end);
}

bool isKingChecked(const ChessBoard board, enum PIECE_COLOR color) {
    PiecePosition kingPosition = {-1, -1};

    // Find the king
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const ChessPiece piece = board.pieces[y][x];

            if (piece.type == PIECE_KING && piece.color == color) {
                kingPosition.x = x;
				kingPosition.y = y;
                break;
            }
        }

        if (kingPosition.x != -1)
            break;
    }

    // No king found
    if (kingPosition.x == -1)
        return false;

    // Check for attacks from opposing pieces
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const ChessPiece piece = board.pieces[y][x];
            if (piece.type == PIECE_NONE || piece.color == color)
                continue;

            PiecePosition pos = {x, y};
            int dx = abs(kingPosition.x - x);
            int dy = abs(kingPosition.y - y);

            switch (piece.type) {
                case PIECE_PAWN: {
                    int direction = piece.color == COLOR_WHITE ? -1 : 1;
                    if (dx == 1 && kingPosition.y - y == direction) return true;
                    break;
                }
                case PIECE_KNIGHT:
                    if (isValidKnightMove(board, pos, kingPosition, true) == MOVE_VALID_CAPTURE) return true;
                    break;
                case PIECE_BISHOP:
                    if (isValidBishopMove(board, pos, kingPosition, true) == MOVE_VALID_CAPTURE) return true;
                    break;
                case PIECE_ROOK:
                    if (isValidRookMove(board, pos, kingPosition, true) == MOVE_VALID_CAPTURE) return true;
                    break;
                case PIECE_QUEEN:
                    if (isValidQueenMove(board, pos, kingPosition, true) == MOVE_VALID_CAPTURE) return true;
                    break;
                case PIECE_KING:
                    if (dx <= 1 && dy <= 1) return true;
                    break;
                default:
                    break;
            }
        }
    }
    return false;
}


// check each piece
// Pawn
enum MoveResult isValidPawnMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing) {
    if (!checkMoveRange(board, start, end))
        return MOVE_INVALID_OUT_OF_RANGE;

    ChessPiece pawn = board.pieces[start.y][start.x];
    ChessPiece target = board.pieces[end.y][end.x];

    if (pawn.type != PIECE_PAWN)
        return MOVE_INVALID_OUT_OF_PIECE_RANGE;

    int direction;
    int startingRank;

    if (pawn.color == COLOR_WHITE) {
        direction = -1;
        startingRank = 6;
    } else if (pawn.color == COLOR_BLACK) {
        direction = 1;
        startingRank = 1;
    } else {
        return MOVE_INVALID;
    }

    int dx = end.x - start.x;
    int dy = end.y - start.y;

    if (dx == 0 && dy == direction) {
        if (target.type != PIECE_NONE)
            return MOVE_INVALID_HAS_OBSTACLE;

        if (!fromKing && !isMoveSafe(board, start, end))
            return MOVE_INVALID_CHECK;

        return MOVE_VALID_MOVE;
    }

    if (dx == 0 && dy == 2 * direction) {
        if (start.y != startingRank || pawn.hasMoved)
            return MOVE_INVALID_HAS_MOVED;

        if (target.type != PIECE_NONE ||
            board.pieces[start.y + direction][start.x].type != PIECE_NONE)
            return MOVE_INVALID_HAS_OBSTACLE;

        if (!fromKing && !isMoveSafe(board, start, end))
            return MOVE_INVALID_CHECK;

        return MOVE_VALID_MOVE;
    }

    if (abs(dx) == 1 && dy == direction) {
        if (target.type == PIECE_NONE) {
            // En passant: capture an adjacent enemy pawn that just advanced two squares.
            ChessPiece adjacent = board.pieces[start.y][end.x];
            if (adjacent.type != PIECE_PAWN || adjacent.color == pawn.color || !adjacent.pawn2Stepped)
                return MOVE_INVALID_OUT_OF_PIECE_RANGE;

            if (!fromKing) {
                ChessBoard tempBoard = board;
                tempBoard.pieces[start.y][start.x].type = PIECE_NONE;
                tempBoard.pieces[start.y][end.x].type = PIECE_NONE;
                tempBoard.pieces[end.y][end.x] = pawn;
                if (isKingChecked(tempBoard, pawn.color))
                    return MOVE_INVALID_CHECK;
            }
            return MOVE_VALID_CAPTURE;
        }

        if (target.color == pawn.color)
            return MOVE_INVALID_SAME_COLOR;

        if (!fromKing && !isCaptureSafe(board, start, end))
            return MOVE_INVALID_CHECK;

        return MOVE_VALID_CAPTURE;
    }

    return MOVE_INVALID_OUT_OF_PIECE_RANGE;
}



// Rook
enum MoveResult isValidRookMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing) {
    if (!checkMoveRange(board, start, end))
        return MOVE_INVALID_OUT_OF_RANGE;

    ChessPiece rook = board.pieces[start.y][start.x];
    ChessPiece target = board.pieces[end.y][end.x];

    int dx = end.x - start.x;
    int dy = end.y - start.y;

    if ((dx != 0 && dy != 0) || (dx == 0 && dy == 0))
        return MOVE_INVALID_OUT_OF_PIECE_RANGE;

    if (target.type != PIECE_NONE && target.color == rook.color)
        return MOVE_INVALID_SAME_COLOR;

    // Determine direction
    int stepX = (dx > 0) - (dx < 0);
    int stepY = (dy > 0) - (dy < 0);

    int x = start.x + stepX;
    int y = start.y + stepY;

    while (x != end.x || y != end.y) {
        if (board.pieces[y][x].type != PIECE_NONE)
            return MOVE_INVALID_HAS_OBSTACLE;

        x += stepX;
        y += stepY;
    }

    if (target.type == PIECE_NONE) {
		if (!fromKing && !isMoveSafe(board, start, end))
			return MOVE_INVALID_CHECK;
        return MOVE_VALID_MOVE;
	}

	if (!fromKing && !isCaptureSafe(board, start, end))
		return MOVE_INVALID_CHECK;

    return MOVE_VALID_CAPTURE;
}


 // Knight
enum MoveResult isValidKnightMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing) {
    if (!checkMoveRange(board, start, end))
        return MOVE_INVALID_OUT_OF_RANGE;

    ChessPiece knight = board.pieces[start.y][start.x];
    ChessPiece target = board.pieces[end.y][end.x];

    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);

    if (!((dx == 2 && dy == 1) || (dx == 1 && dy == 2)))
        return MOVE_INVALID_OUT_OF_PIECE_RANGE;

    if (target.type != PIECE_NONE && target.color == knight.color)
        return MOVE_INVALID_SAME_COLOR;

    if (target.type == PIECE_NONE) {
		if (!fromKing && !isMoveSafe(board, start, end))
			return MOVE_INVALID_CHECK;
        return MOVE_VALID_MOVE;
	}

	if (!fromKing && !isCaptureSafe(board, start, end))
		return MOVE_INVALID_CHECK;
    return MOVE_VALID_CAPTURE;
}


// Bishop
enum MoveResult isValidBishopMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing) {
    if (!checkMoveRange(board, start, end))
        return MOVE_INVALID_OUT_OF_RANGE;

    ChessPiece bishop = board.pieces[start.y][start.x];
    ChessPiece target = board.pieces[end.y][end.x];

    int dx = end.x - start.x;
    int dy = end.y - start.y;

    if (abs(dx) != abs(dy) || (dx == 0 && dy == 0))
        return MOVE_INVALID_OUT_OF_PIECE_RANGE;

    if (target.type != PIECE_NONE && target.color == bishop.color)
        return MOVE_INVALID_SAME_COLOR;

    // Check for obstacles along the diagonal
    int stepX = (dx > 0) ? 1 : -1;
    int stepY = (dy > 0) ? 1 : -1;

    int x = start.x + stepX;
    int y = start.y + stepY;

    while (x != end.x || y != end.y) {
        if (board.pieces[y][x].type != PIECE_NONE)
            return MOVE_INVALID_HAS_OBSTACLE;

        x += stepX;
        y += stepY;
    }

    if (target.type == PIECE_NONE) {
		if (!fromKing && !isMoveSafe(board, start, end))
			return MOVE_INVALID_CHECK;
        return MOVE_VALID_MOVE;
	}

	if (!fromKing && !isCaptureSafe(board, start, end))
		return MOVE_INVALID_CHECK;
    return MOVE_VALID_CAPTURE;
}

// King
enum MoveResult isValidKingMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing) {
    if (!checkMoveRange(board, start, end))
        return MOVE_INVALID_OUT_OF_RANGE;

    ChessPiece king = board.pieces[start.y][start.x];
    ChessPiece target = board.pieces[end.y][end.x];

    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);

    // Must move exactly one square in any direction
	if (!(target.type == PIECE_ROOK && target.color == king.color)) {
		if (dx > 1 || dy > 1 || (dx == 0 && dy == 0))
			return MOVE_INVALID_OUT_OF_PIECE_RANGE;
	} else {
		// Castle
		printf("user tried to castle!\n");

		if (target.type != PIECE_ROOK || target.color != king.color)
			return MOVE_INVALID_CANNOT_CASTLE;
		
		if (isKingChecked(board, king.color))
			return MOVE_INVALID_CANNOT_CASTLE;

		if (king.hasMoved || target.hasMoved)
			return MOVE_INVALID_CANNOT_CASTLE;

		// check direction
		int diff = end.x - start.x;
		if (diff == 3) {
			// on the right
			bool present = isPiecePresent(board, (PiecePosition){start.x + 1, start.y}) ||
						   isPiecePresent(board, (PiecePosition){start.x + 2, start.y});

			if (present)
				return MOVE_INVALID_HAS_OBSTACLE;
			
			// check if obstacle
			bool safe = isMoveSafe(board, start, (PiecePosition){start.x + 1, start.y}) &&
					    isMoveSafe(board, start, (PiecePosition){start.x + 2, start.y});

			if (!safe)
				return MOVE_INVALID_CANNOT_CASTLE;

			return MOVE_VALID_CASTLE;
		} else if (diff == -4) {
			// on the left, queen side
			bool present = isPiecePresent(board, (PiecePosition){start.x - 1, start.y}) ||
						   isPiecePresent(board, (PiecePosition){start.x - 2, start.y}) || 
						   isPiecePresent(board, (PiecePosition){start.x - 3, start.y});

			if (present)
				return MOVE_INVALID_HAS_OBSTACLE;
			
			// check if obstacle
			bool safe = isMoveSafe(board, start, (PiecePosition){start.x - 1, start.y}) &&
					    isMoveSafe(board, start, (PiecePosition){start.x - 2, start.y});

			if (!safe)
				return MOVE_INVALID_CANNOT_CASTLE;

			return MOVE_VALID_CASTLE;
		} else {
			return MOVE_INVALID_CANNOT_CASTLE;
		}
		return MOVE_INVALID_OUT_OF_PIECE_RANGE;
	}
    // Cannot capture a piece of the same color
    if (target.type != PIECE_NONE && target.color == king.color)
        return MOVE_INVALID_SAME_COLOR;

    // Simulate the move
    ChessBoard tempBoard = board;

    tempBoard.pieces[start.y][start.x].type = PIECE_NONE;
    tempBoard.pieces[end.y][end.x] = king;

    // Check whether the destination is attacked
	if(!fromKing && isKingChecked(tempBoard, king.color))
		return MOVE_INVALID_CHECK;

    if (target.type != PIECE_NONE)
        return MOVE_VALID_CAPTURE;

    return MOVE_VALID_MOVE;
}

// Queen
enum MoveResult isValidQueenMove(const ChessBoard board, PiecePosition start, PiecePosition end, bool fromKing) {
    if (!checkMoveRange(board, start, end))
        return MOVE_INVALID_OUT_OF_RANGE;

    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);

    if (dx == 0 || dy == 0)
        return isValidRookMove(board, start, end, fromKing);

    if (dx == dy)
        return isValidBishopMove(board, start, end, fromKing);

    return MOVE_INVALID_OUT_OF_PIECE_RANGE;

}

int GetAvailableMoves(const ChessBoard board, PiecePosition positionsBuffer[64], PiecePosition pos){
	// ideally, only call once per move, not per frame
	ChessPiece piece = getPieceAt(board, pos);
	memset(positionsBuffer, 0, sizeof(PiecePosition) * 64);

	int currentSize = 0;
	for(int y = 0; y < 8; ++y) {
		for(int x = 0; x < 8; ++x) {
            switch (piece.type) {
                case PIECE_PAWN:
					if (isValidPawnMove(board, pos, (PiecePosition){x, y}, false) >= MOVE_VALID)
						positionsBuffer[currentSize++] = (PiecePosition){x, y}, false;
                    break;
                case PIECE_KNIGHT:
					if (isValidKnightMove(board, pos, (PiecePosition){x, y}, false) >= MOVE_VALID)
						positionsBuffer[currentSize++] = (PiecePosition){x, y}, false;
                    break;
                case PIECE_BISHOP:
					if (isValidBishopMove(board, pos, (PiecePosition){x, y}, false) >= MOVE_VALID)
						positionsBuffer[currentSize++] = (PiecePosition){x, y}, false;
                    break;
                case PIECE_ROOK:
					if (isValidRookMove(board, pos, (PiecePosition){x, y}, false) >= MOVE_VALID)
						positionsBuffer[currentSize++] = (PiecePosition){x, y}, false;
                    break;
                case PIECE_QUEEN:
					if (isValidQueenMove(board, pos, (PiecePosition){x, y}, false) >= MOVE_VALID)
						positionsBuffer[currentSize++] = (PiecePosition){x, y}, false;
                    break;
                case PIECE_KING:
					if (isValidKingMove(board, pos, (PiecePosition){x, y}, false) >= MOVE_VALID)
						positionsBuffer[currentSize++] = (PiecePosition){x, y}, false;
                    break;
                default:
                    break;
            }
		}
	}

	return currentSize;
}

bool ifCheckmate(const ChessBoard board, enum PIECE_COLOR color){
	if(!isKingChecked(board, color))
		return false;	

	PiecePosition positionBuffer[64];
	for(int x = 0; x < 8; x++){
		for(int y = 0; y < 8; y++){
			ChessPiece test_piece = board.pieces[x][y];
			if(test_piece.color == color){
				switch (test_piece.type) {
					case PIECE_PAWN:
					case PIECE_KNIGHT:
					case PIECE_BISHOP:
					case PIECE_ROOK:
					case PIECE_QUEEN:
					case PIECE_KING:
						if(GetAvailableMoves(board, positionBuffer, (PiecePosition){x, y}) > 0) return true;
						break;
					default:
						break;
				}
			}
		}
		memset(positionBuffer, 0, sizeof(positionBuffer));
	}

	return true;
}

bool ifStalemate(const ChessBoard board, enum PIECE_COLOR color){
	if(isKingChecked(board, color))
		return false;

	PiecePosition positionBuffer[64];
	for(int x = 0; x < 8; x++){
		for(int y = 0; y < 8; y++){
			ChessPiece test_piece = board.pieces[x][y];
			if(test_piece.color == color){
				switch (test_piece.type) {
					case PIECE_PAWN:
					case PIECE_KNIGHT:
					case PIECE_BISHOP:
					case PIECE_ROOK:
					case PIECE_QUEEN:
					case PIECE_KING:
						if(GetAvailableMoves(board, positionBuffer, (PiecePosition){x, y}) > 0) return false;
						break;
					default:
						break;
				}
			}
		}
		memset(positionBuffer, 0, sizeof(positionBuffer));
	}

	return true;
}
