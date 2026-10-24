#include "../include/render_assets.h"
#include <algorithm>
#include <exception>
#include <vector>
#include "../vendor/raylib/include/raygui.h"

#define WHITE_SQUARE_COLOR (GetColor(0xa8a296FF))
#define BLACK_SQUARE_COLOR (GetColor(0x445240FF))

PiecePosition toBoardPosition(int x, int y, PIECE_COLOR playerColor) {
	if(playerColor == COLOR_BLACK) {
		x = 7 - x;
		y = 7 - y;
	}
	return {x, y};
}

void renderBoard(const GameState& gameState, RenderInfo info) {
	if(!gameState.gameStarted || (info.playerColor != COLOR_WHITE && info.playerColor != COLOR_BLACK))
		return;

	float squareSize = GetScreenHeight()/10.0f;
	int pixelSize = squareSize/15;
	float xStart = squareSize, yStart = squareSize;

	// uhhh render the border of the chessboard
	DrawRectangleLinesEx({xStart - 5.0f, yStart - 5.0f, squareSize * 8 + 10.0f, squareSize * 8 + 10.0f}, 10.0f, BLACK);

	// possible positions given a chosen cell/square
	// static so it doesn't recompute ( i think )
	static std::vector<PiecePosition> positions;
	static PiecePosition cachedStart = {-1, -1};
	static PIECE_COLOR cachedTurn = COLOR_NONE;

	bool hasSelection = info.chosenStart.x >= 0 && info.chosenStart.y >= 0;
	if(!hasSelection || info.chosenStart.x != cachedStart.x || info.chosenStart.y != cachedStart.y || gameState.currentColorTurn != cachedTurn) {
		positions.clear();
		cachedStart = hasSelection ? info.chosenStart : PiecePosition{-1, -1};
		cachedTurn = gameState.currentColorTurn;
		if(hasSelection && getPieceAt(gameState.currentChessBoard, info.chosenStart).color == info.playerColor 
				&& getPieceAt(gameState.currentChessBoard, info.chosenStart).type != PIECE_NONE
				&& gameState.currentColorTurn == info.playerColor) {
			positions = GetAvailableMoves(gameState.currentChessBoard, info.chosenStart);
		}
	}

	Vector2 mouse = GetMousePosition();
	int hoverX = (int)((mouse.x - squareSize) / squareSize);
	int hoverY = (int)((mouse.y - squareSize) / squareSize);
	bool hoveringBoard = hoverX >= 0 && hoverX < 8 && hoverY >= 0 && hoverY < 8
						 && gameState.currentColorTurn == info.playerColor;

	for(int y = 0; y < 8; ++y) {
		for(int x = 0; x < 8; ++x) {
			PiecePosition boardPosition = toBoardPosition(x, y, info.playerColor);
			Color cellColor = (x + y) % 2 ? BLACK_SQUARE_COLOR : WHITE_SQUARE_COLOR;
			DrawRectangle(xStart, yStart, squareSize, squareSize, cellColor);

			if(std::find_if(positions.begin(), positions.end(), [boardPosition](const PiecePosition& pos) {
				return pos.x == boardPosition.x && pos.y == boardPosition.y;
			}) != positions.end()) {
				DrawRectangle(xStart, yStart, squareSize, squareSize, ColorLerp(cellColor, GREEN, 0.25f));
			}

			if(hoveringBoard && hoverX == x && hoverY == y) {
				DrawRectangle(xStart, yStart, squareSize, squareSize, ColorLerp(cellColor, YELLOW, 0.25f));
			}

			if(info.chosenStart.x == boardPosition.x && info.chosenStart.y == boardPosition.y
					&& gameState.currentColorTurn == info.playerColor) {
				DrawRectangle(xStart, yStart, squareSize, squareSize, ColorLerp(cellColor, GREEN, 0.5f));
			}

			ChessPiece piece = getPieceAt(gameState.currentChessBoard, boardPosition);
			if(piece.type == PIECE_KING && gameState.checkedColor == piece.color)
				DrawRectangle(xStart, yStart, squareSize, squareSize, RED);

			if(piece.type != PIECE_NONE)
				GuiDrawIcon(piece.type + 256, (int)xStart, (int)yStart, pixelSize, piece.color == COLOR_WHITE ? WHITE : BLACK);

			xStart += squareSize;
		}
		xStart = squareSize;
		yStart += squareSize;
	}
}
