#include "../../include/chess/chess_board.h"
#include <stdio.h>

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

bool movePiece(ChessBoard& board, PiecePosition start, PiecePosition end) {
	if(!checkRange(board, start, end))
		return false;

	bool isValid = false;
	ChessPiece& piece = board.pieces[start.y][start.x];
	ChessPiece& target = board.pieces[end.y][end.x];
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
		if(isValid) piece.hasMoved = true;
		if(isValid) std::swap(piece, target);
	//

	return isValid;
}
