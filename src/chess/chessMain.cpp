#include "../../include/chess/chess_board.h"
#include <stdio.h>

int main(void) {
    ChessBoard board = standardBoard;
    int turn = 0; // 0 = White, 1 = Black

    printf("=== Chess 1v1 Test ===\n");
    printf("Enter moves as: start_x start_y end_x end_y\n");
    printf("Coordinates range from 0 to 7. Enter -1 to quit.\n\n");

    print_board(board);

    while (1) {
        int sx, sy, ex, ey;
        printf("\n%s's turn\n", turn == 0 ? "White" : "Black");
        printf("Move: ");

        if (scanf("%d", &sx) != 1)
            break;

        if (sx == -1)
            break;

        if (scanf("%d %d %d", &sy, &ex, &ey) != 3) {
            printf("Invalid input.\n");
            break;
        }

        PiecePosition start = {sx, sy};
        PiecePosition end = {ex, ey};

        MoveResult result = movePiece(board, start, end);

        if (result >= MOVE_VALID) {
            printf("Move accepted (result: %d).\n", result);
            print_board(board);
            turn = !turn;
        } else {
            printf("Invalid move (result: %d). Try again.\n", result);
        }
    }

    printf("\nGame ended.\n");
    return 0;
}

