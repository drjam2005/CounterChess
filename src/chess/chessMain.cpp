#include "../../include/chess/chess_board.h"
#include <stdio.h>

int main(void) {
	ChessBoard board = standardBoard;
	printf("size of board: %lu\n", sizeof board);

	if(movePiece(board, {1, 6}, {1, 4}) >= MOVE_VALID) {
		printf("valid\n");
	} else {
		printf("invalid\n");
	}

	print_board(board);

	printf("\n");
	if(movePiece(board, {1, 4}, {1, 2}) >= MOVE_VALID) {
		printf("valid\n");
	} else {
		printf("invalid\n");
	}
	printf("\n");

	print_board(board);
	printf("\n");

}
