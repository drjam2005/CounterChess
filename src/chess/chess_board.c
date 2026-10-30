#include "../../include/chess/chess_board.h"
#include <stdio.h>

// Same standard starting position as the original chess_board.cpp.
// All movement flags are initially false (zero-initialized).
const ChessBoard standardBoard = {
    .pieces = {
        {
            {.color = COLOR_BLACK, .type = PIECE_ROOK, .previousType = PIECE_ROOK},
            {.color = COLOR_BLACK, .type = PIECE_KNIGHT, .previousType = PIECE_KNIGHT},
            {.color = COLOR_BLACK, .type = PIECE_BISHOP, .previousType = PIECE_BISHOP},
            {.color = COLOR_BLACK, .type = PIECE_QUEEN, .previousType = PIECE_QUEEN},
            {.color = COLOR_BLACK, .type = PIECE_KING, .previousType = PIECE_KING},
            {.color = COLOR_BLACK, .type = PIECE_BISHOP, .previousType = PIECE_BISHOP},
            {.color = COLOR_BLACK, .type = PIECE_KNIGHT, .previousType = PIECE_KNIGHT},
            {.color = COLOR_BLACK, .type = PIECE_ROOK, .previousType = PIECE_ROOK}
        },
        {
            {.color = COLOR_BLACK, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_BLACK, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_BLACK, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_BLACK, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_BLACK, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_BLACK, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_BLACK, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_BLACK, .type = PIECE_PAWN, .previousType = PIECE_PAWN}
        },
        {
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE}
        },
        {
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE}
        },
        {
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE}
        },
        {
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE},
            {.color = COLOR_NONE, .type = PIECE_NONE, .previousType = PIECE_NONE}
        },
        {
            {.color = COLOR_WHITE, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_WHITE, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_WHITE, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_WHITE, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_WHITE, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_WHITE, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_WHITE, .type = PIECE_PAWN, .previousType = PIECE_PAWN},
            {.color = COLOR_WHITE, .type = PIECE_PAWN, .previousType = PIECE_PAWN}
        },
        {
            {.color = COLOR_WHITE, .type = PIECE_ROOK, .previousType = PIECE_ROOK},
            {.color = COLOR_WHITE, .type = PIECE_KNIGHT, .previousType = PIECE_KNIGHT},
            {.color = COLOR_WHITE, .type = PIECE_BISHOP, .previousType = PIECE_BISHOP},
            {.color = COLOR_WHITE, .type = PIECE_QUEEN, .previousType = PIECE_QUEEN},
            {.color = COLOR_WHITE, .type = PIECE_KING, .previousType = PIECE_KING},
            {.color = COLOR_WHITE, .type = PIECE_BISHOP, .previousType = PIECE_BISHOP},
            {.color = COLOR_WHITE, .type = PIECE_KNIGHT, .previousType = PIECE_KNIGHT},
            {.color = COLOR_WHITE, .type = PIECE_ROOK, .previousType = PIECE_ROOK}
        }
    }
};

// Preserve the existing char* signature for compatibility with chess_board.h.
// Use writable arrays instead of casting away const from string literals.
char* PieceToString(enum PIECE_TYPE type) {
    static char pawn[] = "PA", rook[] = "RO", knight[] = "KN";
    static char bishop[] = "BI", queen[] = "QU", king[] = "KI";
    static char none[] = "NO", unknown[] = "UN";

    switch (type) {
        case PIECE_PAWN:   return pawn;
        case PIECE_ROOK:   return rook;
        case PIECE_KNIGHT: return knight;
        case PIECE_BISHOP: return bishop;
        case PIECE_QUEEN:  return queen;
        case PIECE_KING:   return king;
        case PIECE_NONE:   return none;
        default:           return unknown;
    }
}

void print_board(const ChessBoard board) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            ChessPiece piece = board.pieces[y][x];
            char symbol = '?';

            switch (piece.type) {
                case PIECE_PAWN:   symbol = 'P'; break;
                case PIECE_ROOK:   symbol = 'R'; break;
                case PIECE_KNIGHT: symbol = 'N'; break;
                case PIECE_BISHOP: symbol = 'B'; break;
                case PIECE_QUEEN:  symbol = 'Q'; break;
                case PIECE_KING:   symbol = 'K'; break;
                case PIECE_NONE:
                    symbol = ((x + y) % 2) ? '_' : '#';
                    break;
                default:
                    break;
            }

            if (piece.type != PIECE_NONE && piece.color == COLOR_BLACK &&
                symbol >= 'A' && symbol <= 'Z') {
                symbol = (char)(symbol - 'A' + 'a');
            }

            printf("%c", symbol);
        }
        printf("\n");
    }
}

// Execute a move ONLY after the corresponding chess_moves.cpp validator
// accepts it. This function retains the project's convention that castling
// is requested by moving the king onto its friendly corner rook.
enum MoveResult movePiece(ChessBoard* board, PiecePosition start, PiecePosition end) {
    if (board == nullptr)
        return MOVE_INVALID;

    if (!checkMoveRange(*board, start, end))
        return MOVE_INVALID_OUT_OF_RANGE;

    if (start.x == end.x && start.y == end.y)
        return MOVE_INVALID;

    ChessPiece piece = board->pieces[start.y][start.x];
    ChessPiece target = board->pieces[end.y][end.x];

    if (piece.type == PIECE_NONE || piece.type == PIECE_INVALID ||
        piece.color == COLOR_NONE)
        return MOVE_INVALID;

    enum MoveResult result = MOVE_INVALID;
    switch (piece.type) {
        case PIECE_PAWN:
            result = isValidPawnMove(*board, start, end, false);
            break;
        case PIECE_KNIGHT:
            result = isValidKnightMove(*board, start, end, false);
            break;
        case PIECE_BISHOP:
            result = isValidBishopMove(*board, start, end, false);
            break;
        case PIECE_ROOK:
            result = isValidRookMove(*board, start, end, false);
            break;
        case PIECE_KING:
            result = isValidKingMove(*board, start, end, false);
            break;
        case PIECE_QUEEN:
            result = isValidQueenMove(*board, start, end, false);
            break;
        default:
            return MOVE_INVALID;
    }

    const bool castle = (result == MOVE_VALID_CASTLE);
    if (!castle && result < MOVE_VALID)
        return result;

    // Keep these safeguards even if a validator changes later.
    if (!castle && target.type != PIECE_NONE &&
        (target.color == piece.color || target.type == PIECE_KING))
        return MOVE_INVALID;

    ChessPiece empty = {};
    empty.color = COLOR_NONE;
    empty.type = PIECE_NONE;
    empty.previousType = PIECE_NONE;

    bool enPassant = false;
    bool pawnDoubleStep = false;

    if (castle) {
        // The king targets the original rook square (a-file or h-file).
        const int homeRow = (piece.color == COLOR_WHITE) ? 7 : 0;
        const int diff = end.x - start.x;
        if (piece.type != PIECE_KING || start.x != 4 ||
            start.y != homeRow || end.y != homeRow ||
            target.type != PIECE_ROOK || target.color != piece.color ||
            piece.hasMoved || target.hasMoved ||
            !((diff == 3 && end.x == 7) || (diff == -4 && end.x == 0)))
            return MOVE_INVALID_CANNOT_CASTLE;
    } else if (piece.type == PIECE_PAWN) {
        const int pawnDirection = (piece.color == COLOR_WHITE) ? -1 : 1;
        pawnDoubleStep = (end.y - start.y == 2 * pawnDirection);
        enPassant = (start.x != end.x && target.type == PIECE_NONE);

        if (enPassant) {
            // An empty diagonal destination is only legal when capturing
            // an adjacent enemy pawn that just advanced two squares.
            ChessPiece adjacent = board->pieces[start.y][end.x];
            if (adjacent.type != PIECE_PAWN ||
                adjacent.color == piece.color || !adjacent.pawn2Stepped)
                return MOVE_INVALID;
        }
    }

    // En passant eligibility lasts for exactly the opponent's next move.
    // Do this only after all move validation has succeeded, including castling.
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            if (board->pieces[y][x].type == PIECE_PAWN)
                board->pieces[y][x].pawn2Stepped = false;
        }
    }

    if (castle) {
        ChessPiece king = piece;
        ChessPiece rook = target;
        king.hasMoved = true;
        rook.hasMoved = true;

        board->pieces[start.y][start.x] = empty;
        board->pieces[end.y][end.x] = empty;

        if (end.x > start.x) {
            board->pieces[start.y][6] = king;  // King: e-file -> g-file
            board->pieces[start.y][5] = rook;  // Rook: h-file -> f-file
        } else {
            board->pieces[start.y][2] = king;  // King: e-file -> c-file
            board->pieces[start.y][3] = rook;  // Rook: a-file -> d-file
        }
    } else {
        if (enPassant)
            board->pieces[start.y][end.x] = empty;

        piece.hasMoved = true;
        piece.pawn2Stepped = pawnDoubleStep;

        // Simple default promotion: pawn -> queen. Its previousType is
        // intentionally retained as PAWN for any tracking that uses it.
        if (piece.type == PIECE_PAWN &&
            ((piece.color == COLOR_WHITE && end.y == 0) ||
             (piece.color == COLOR_BLACK && end.y == 7))) {
            piece.type = PIECE_QUEEN;
            piece.pawn2Stepped = false;
        }

        board->pieces[start.y][start.x] = empty;
        board->pieces[end.y][end.x] = piece;
    }

    return result;
}
