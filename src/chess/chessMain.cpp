#include "../../include/chess/chess_board.h"
#include <stdio.h>

int main(void) {
	ChessBoard board = standardBoard;

	print_board(board);

	if(movePiece(board, {1, 6}, {1, 4}))
		printf("valid\n");

	printf("\n");

	print_board(board);
}
