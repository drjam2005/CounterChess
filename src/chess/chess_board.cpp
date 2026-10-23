#include "../../include/chess/chess_board.h"
#include <stdio.h>

std::string PieceToString(PIECE_TYPE type) {
    switch (type) {
        case PIECE_PAWN:   return "PA";
        case PIECE_ROOK:   return "RO";
        case PIECE_KNIGHT: return "KN";
        case PIECE_BISHOP: return "BI";
        case PIECE_QUEEN:  return "QU";
        case PIECE_KING:   return "KI";
        case PIECE_NONE:   return "NO";
        default:           return "UN";
    }
}

void print_board(const ChessBoard board) {
	for(int y = 0; y < 8; ++y) {
		for(int x = 0; x < 8; ++x) {
			ChessPiece piece = board.pieces[y][x];
			
			if( piece.type != PIECE_NONE) {
				if( piece.color == COLOR_WHITE ) {
					if( piece.type == PIECE_PAWN )
						printf("P");
					if( piece.type == PIECE_ROOK )
						printf("R");
					if( piece.type == PIECE_KNIGHT )
						printf("N");
					if( piece.type == PIECE_BISHOP )
						printf("B");
					if( piece.type == PIECE_QUEEN )
						printf("Q");
					if( piece.type == PIECE_KING )
						printf("K");
				} else if( piece.color == COLOR_BLACK) {
					if( piece.type == PIECE_PAWN )
						printf("p");
					if( piece.type == PIECE_ROOK )
						printf("r");
					if( piece.type == PIECE_KNIGHT )
						printf("n");
					if( piece.type == PIECE_BISHOP )
						printf("b");
					if( piece.type == PIECE_QUEEN )
						printf("q");
					if( piece.type == PIECE_KING )
						printf("k");
				}
			} else {
				if((x + y) % 2) printf("_");
				else printf("#");
			}
			printf("");
		}
		printf("\n");
	}
}

MoveResult movePiece(ChessBoard& board, PiecePosition start, PiecePosition end) {
	if(!checkMoveRange(board, start, end))
		return MOVE_INVALID_OUT_OF_RANGE;

	MoveResult isValid = MOVE_INVALID;
	ChessPiece piece = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	switch (piece.type) {
		case PIECE_PAWN:
			isValid = isValidPawnMove(board, start, end);
			break;
		case PIECE_KNIGHT:
			isValid = isValidKnightMove(board, start, end);
			break;
		case PIECE_BISHOP:
			isValid = isValidBishopMove(board, start, end);
			break;
		case PIECE_ROOK:
			isValid = isValidRookMove(board, start, end);
			break;
		case PIECE_KING:
			isValid = isValidKingMove(board, start, end);
			break;
		case PIECE_QUEEN:
			isValid = isValidQueenMove(board, start, end);
			break;
		default:
			break;
	}

	// (FOR TESTING)
	if (isValid == MOVE_VALID_CASTLE) {
		int diff = end.x - start.x;
		ChessPiece king = board.pieces[start.y][start.x];
		ChessPiece rook = board.pieces[end.y][end.x];
		board.pieces[start.y][start.x].type = PIECE_NONE;
		board.pieces[end.y][end.x].type = PIECE_NONE;

		if (diff == 3) {
			board.pieces[start.y][6] = king;
			board.pieces[start.y][5] = rook;
		} else if (diff == -4) {
			board.pieces[start.y][2] = king;
			board.pieces[start.y][3] = rook;
		}
	} else if (isValid >= MOVE_VALID) {
		bool enPassant = piece.type == PIECE_PAWN && start.x != end.x && target.type == PIECE_NONE;
		int pawnDirection = piece.color == COLOR_WHITE ? -1 : 1;
		bool pawnDoubleStep = piece.type == PIECE_PAWN && end.y - start.y == 2 * pawnDirection;

		// The right to capture en passant lasts for one move only.
		for (int y = 0; y < 8; ++y) {
			for (int x = 0; x < 8; ++x) {
				if (board.pieces[y][x].type == PIECE_PAWN)
					board.pieces[y][x].pawn2Stepped = false;
			}
		}

		if (enPassant)
			board.pieces[start.y][end.x].type = PIECE_NONE;

		piece.hasMoved = true;
		piece.pawn2Stepped = pawnDoubleStep;
		board.pieces[start.y][start.x].type = PIECE_NONE;
		board.pieces[end.y][end.x] = piece;
	}
	//

	return isValid;
}
