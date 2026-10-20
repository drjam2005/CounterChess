#include "../../include/chess/chess_board.h"

bool checkRange(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if (start.x < 0 || end.x < 0 || start.x >= 8 || end.x >= 8)
		return false;
	if (start.y < 0 || end.y < 0 || start.y >= 8 || end.y >= 8)
		return false;

	return true;
}

// check each piece
// Pawn
bool isValidPawnMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkRange(board, start, end))
		return false;

	ChessPiece pawn = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	return true;
}

// Rook
bool isValidRookMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkRange(board, start, end))
		return false;

	ChessPiece rook = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	return true;
}

// Knight
bool isValidKnightMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkRange(board, start, end))
		return false;

	ChessPiece knight = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	return true;
}

// Bishop
bool isValidBishopMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkRange(board, start, end))
		return false;

	ChessPiece bishop = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	return true;
}

// King
bool isValidKingMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkRange(board, start, end))
		return false;

	ChessPiece king = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	return true;
}

// Queen
bool isValidQueenMove(const ChessBoard board, PiecePosition start, PiecePosition end) {
	if(!checkRange(board, start, end))
		return false;

	ChessPiece queen = board.pieces[start.y][start.x];
	ChessPiece target = board.pieces[end.y][end.x];
	return true;
}
