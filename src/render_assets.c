#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../include/render_assets.h"
#include "../vendor/raylib/include/raygui.h"
#include "../vendor/alpha/piece_assets.h"

#define WHITE_SQUARE_COLOR (GetColor(0xa8a296FF))
#define BLACK_SQUARE_COLOR (GetColor(0x445240FF))
#define BOARD_X_OFFSET 200.0f
#define UI_X_OFFSET 200
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

struct PiecePosition positions[64];
int positionsCount = 0;
static struct PiecePosition cachedStart = {-1, -1};
static enum PIECE_COLOR cachedTurn = COLOR_NONE;
static Texture2D pieceTextures[COLOR_COUNT][PIECE_COUNT];
static bool pieceTexturesLoaded = false;

static Texture2D loadPieceTexture(const unsigned char *data, int dataSize) {
	Image image = LoadImageFromMemory(".png", data, dataSize);
	Texture2D texture = LoadTextureFromImage(image);
	UnloadImage(image);
	return texture;
}

void loadPieceTextures(void) {
	if (pieceTexturesLoaded)
		return;

	pieceTextures[COLOR_WHITE][PIECE_PAWN] = loadPieceTexture(wp_png, wp_png_len);
	pieceTextures[COLOR_WHITE][PIECE_KNIGHT] = loadPieceTexture(wn_png, wn_png_len);
	pieceTextures[COLOR_WHITE][PIECE_BISHOP] = loadPieceTexture(wb_png, wb_png_len);
	pieceTextures[COLOR_WHITE][PIECE_ROOK] = loadPieceTexture(wr_png, wr_png_len);
	pieceTextures[COLOR_WHITE][PIECE_KING] = loadPieceTexture(wk_png, wk_png_len);
	pieceTextures[COLOR_WHITE][PIECE_QUEEN] = loadPieceTexture(wq_png, wq_png_len);
	pieceTextures[COLOR_BLACK][PIECE_PAWN] = loadPieceTexture(bp_png, bp_png_len);
	pieceTextures[COLOR_BLACK][PIECE_KNIGHT] = loadPieceTexture(bn_png, bn_png_len);
	pieceTextures[COLOR_BLACK][PIECE_BISHOP] = loadPieceTexture(bb_png, bb_png_len);
	pieceTextures[COLOR_BLACK][PIECE_ROOK] = loadPieceTexture(br_png, br_png_len);
	pieceTextures[COLOR_BLACK][PIECE_KING] = loadPieceTexture(bk_png, bk_png_len);
	pieceTextures[COLOR_BLACK][PIECE_QUEEN] = loadPieceTexture(bq_png, bq_png_len);
	pieceTexturesLoaded = true;
}

void unloadPieceTextures(void) {
	if (!pieceTexturesLoaded)
		return;
	for (int color = COLOR_WHITE; color <= COLOR_BLACK; ++color) {
		for (int type = PIECE_PAWN; type <= PIECE_QUEEN; ++type) {
			if (pieceTextures[color][type].id != 0)
				UnloadTexture(pieceTextures[color][type]);
		}
	}
	pieceTexturesLoaded = false;
}

struct PiecePosition toBoardPosition(int x, int y, enum PIECE_COLOR playerColor) {
	if(playerColor == COLOR_BLACK) {
		x = 7 - x;
		y = 7 - y;
	}
	return (struct PiecePosition){x, y};
}

void renderBoard(const struct GameState gameState, struct RenderInfo info) {
	if(!gameState.gameStarted || (info.playerColor != COLOR_WHITE && info.playerColor != COLOR_BLACK))
		return;

	float squareSize = GetScreenHeight()/10.0f;
	float xStart = squareSize + BOARD_X_OFFSET, yStart = squareSize;

	// uhhh render the border of the chessboard
	DrawRectangleLinesEx((Rectangle){xStart - 5.0f, yStart - 5.0f, squareSize * 8 + 10.0f, squareSize * 8 + 10.0f}, 10.0f, BLACK);

	bool hasSelection = info.chosenStart.x >= 0 && info.chosenStart.y >= 0;
	if(!hasSelection || info.chosenStart.x != cachedStart.x || info.chosenStart.y != cachedStart.y || gameState.currentColorTurn != cachedTurn) {

		positionsCount = 0;

		cachedStart = hasSelection ? info.chosenStart : (PiecePosition){-1, -1};
		cachedTurn = gameState.currentColorTurn;

		if(hasSelection && getPieceAt(gameState.currentChessBoard, info.chosenStart).color == info.playerColor 
				&& getPieceAt(gameState.currentChessBoard, info.chosenStart).type != PIECE_NONE
				&& gameState.currentColorTurn == info.playerColor) {
			positionsCount = GetAvailableMoves(gameState.currentChessBoard, positions, info.chosenStart);
		}
	}

	Vector2 mouse = GetMousePosition();
	int hoverX = (int)((mouse.x - squareSize - BOARD_X_OFFSET) / squareSize);
	int hoverY = (int)((mouse.y - squareSize) / squareSize);
	bool hoveringBoard = hoverX >= 0 && hoverX < 8 && hoverY >= 0 && hoverY < 8
						 && gameState.currentColorTurn == info.playerColor;


	for(int y = 0; y < 8; ++y) {
		for(int x = 0; x < 8; ++x) {
			struct PiecePosition boardPosition = toBoardPosition(x, y, info.playerColor);
			Color cellColor = (x + y) % 2 ? BLACK_SQUARE_COLOR : WHITE_SQUARE_COLOR;
			DrawRectangle(xStart, yStart, squareSize, squareSize, cellColor);

			// for loop to find
			bool found = false;
			for(int i = 0; i < positionsCount; ++i) {
				if(positions[i].x == boardPosition.x && positions[i].y == boardPosition.y)
					found = true;
			}

			

			if(hoveringBoard && hoverX == x && hoverY == y) {
				DrawRectangle(xStart, yStart, squareSize, squareSize, ColorLerp(cellColor, YELLOW, 0.25f));
			}

			ChessPiece piece = getPieceAt(gameState.currentChessBoard, boardPosition);
			if(piece.type != PIECE_NONE && (piece.type != piece.previousType && piece.color != piece.previousColor)){
				DrawRectangle(xStart, yStart, squareSize, squareSize, ColorLerp(cellColor, ORANGE, 0.50f));
			} 

			if(piece.type == PIECE_NONE && (piece.type != piece.previousType)){
				DrawRectangle(xStart, yStart, squareSize, squareSize, ColorLerp(cellColor, ORANGE, 0.25f));
			} 

			if (found)
				DrawRectangle(xStart, yStart, squareSize, squareSize, ColorLerp(cellColor, GREEN, 0.25f));

			if(info.chosenStart.x == boardPosition.x && info.chosenStart.y == boardPosition.y
					&& gameState.currentColorTurn == info.playerColor) {
				DrawRectangle(xStart, yStart, squareSize, squareSize, ColorLerp(cellColor, GREEN, 0.5f));
			}

			if(piece.type == PIECE_KING && gameState.checkedColor == piece.color)
				DrawRectangle(xStart, yStart, squareSize, squareSize, RED);
			xStart += squareSize;
		}
		xStart = squareSize + BOARD_X_OFFSET;
		yStart += squareSize;
	}

	xStart = squareSize + BOARD_X_OFFSET, yStart = squareSize;
	for(int y = 0; y < 8; ++y) {
		for(int x = 0; x < 8; ++x) {
			struct PiecePosition boardPosition = toBoardPosition(x, y, info.playerColor);
			ChessPiece piece = getPieceAt(gameState.currentChessBoard, boardPosition);
			if (piece.type >= PIECE_PAWN && piece.type <= PIECE_QUEEN &&
					piece.color >= COLOR_WHITE && piece.color <= COLOR_BLACK) {
				Texture2D tex = pieceTextures[piece.color][piece.type];
				if (tex.id != 0) {
					if(info.chosenStart.x == boardPosition.x && info.chosenStart.y == boardPosition.y && !info.isPromoting && getPieceAt(gameState.currentChessBoard, boardPosition).color == info.playerColor) {
						Vector2 mousePos = GetMousePosition();
						//GuiDrawIcon(piece.type + 256, mousePos.x - (8*pixelSize), mousePos.y - (8*pixelSize), pixelSize, piece.color == COLOR_WHITE ? WHITE : BLACK);
						DrawTexturePro(tex, (Rectangle){0, 0, (float)tex.width, (float)tex.height},
							(Rectangle){mousePos.x , mousePos.y , squareSize, squareSize},
							(Vector2){squareSize / 2.0f, squareSize / 2.0f}, 0.0f, WHITE);
					} else {
						DrawTexturePro(tex, (Rectangle){0, 0, (float)tex.width, (float)tex.height},
								(Rectangle){xStart + (squareSize/2.0f), yStart + (squareSize/2.0f), squareSize, squareSize},
								(Vector2){squareSize / 2.0f, squareSize / 2.0f}, 0.0f, WHITE);
					}
					// x/y are screen coordinates here; boardPosition is the actual
					// square after applying the current player's perspective.
					// Use the piece already looked up at that square so the outline
					// follows the same piece in either orientation.
					if(piece.previousType != PIECE_NONE && piece.previousType != piece.type && piece.previousColor == piece.color){
						DrawRectangleLinesEx((Rectangle){xStart, yStart, squareSize, squareSize}, 2.0f, RED);
					}
				}
			}

			xStart += squareSize;
		}
		xStart = squareSize + BOARD_X_OFFSET;
		yStart += squareSize;
	}

	for(int x = 0; x < 8; ++x) {
		DrawText(TextFormat("%c", info.playerColor == COLOR_WHITE ? 'a'+x : 'h'-x), BOARD_X_OFFSET + squareSize+squareSize/2.0f+(squareSize*x)-10, squareSize/2.0f + (squareSize*9)-10, 20, BLACK);
	}
	for(int y = 0; y < 8; ++y) {
		DrawText(TextFormat("%d", info.playerColor == COLOR_WHITE ? 1+y : 8-y), BOARD_X_OFFSET + squareSize/2.0f, squareSize+squareSize/2.0f+(squareSize*(7-y))-5, 20, BLACK);
	}
}

char* fitMessage(const char* message, int font_size, float maxWidth, float* resultingHeight) {
    if (!message || maxWidth <= 0 || font_size <= 0)
        return NULL;

    Font font = GetFontDefault();
    float spacing = (float)(font_size / 10 > 0 ? font_size / 10 : 1);
    size_t len = strlen(message);

    if (len > (SIZE_MAX - 1) / 3)
        return NULL;

    char* msg = malloc(len * 3 + 1);
    char* temp = malloc(len + 2);

    if (!msg || !temp) {
        free(msg);
        free(temp);
        return NULL;
    }

    size_t out = 0;
    const char* remaining = message;

    while (*remaining) {
        // Skip leading spaces after wrapping
        while (*remaining == ' ' || *remaining == '\t')
            remaining++;

        if (!*remaining)
            break;

        // Respect existing newlines
        if (*remaining == '\n') {
            msg[out++] = '\n';
            remaining++;
            continue;
        }

        size_t remainingLen = strcspn(remaining, "\n");
        size_t fit = 0;
        size_t lastSpace = 0;

        // Find the longest substring that fits
        for (size_t i = 1; i <= remainingLen; i++) {
            memcpy(temp, remaining, i);
            temp[i] = '\0';

            if (MeasureTextEx(font, temp, font_size, spacing).x > maxWidth)
                break;

            fit = i;

            if (remaining[i - 1] == ' ' || remaining[i - 1] == '\t')
                lastSpace = i;
        }

        // Entire line fits
        if (fit == remainingLen) {
            memcpy(msg + out, remaining, fit);
            out += fit;
            remaining += fit;
            continue;
        }

        // Prefer wrapping at whitespace
        if (lastSpace > 0) {
            size_t count = lastSpace;

            while (count > 0 &&
                   (remaining[count - 1] == ' ' ||
                    remaining[count - 1] == '\t'))
                count--;

            memcpy(msg + out, remaining, count);
            out += count;
            msg[out++] = '\n';

            remaining += lastSpace;
            continue;
        }

        // No whitespace fits: split word with a hyphen
        size_t split = fit;

        // Make sure the hyphen also fits
        while (split > 0) {
            memcpy(temp, remaining, split);
            temp[split] = '-';
            temp[split + 1] = '\0';

            if (MeasureTextEx(font, temp, font_size, spacing).x <= maxWidth)
                break;

            split--;
        }

        // Fallback for extremely narrow widths
        if (split == 0) {
            split = 1;
            memcpy(msg + out, remaining, split);
            out += split;
            remaining += split;
            if (*remaining)
                msg[out++] = '\n';
            continue;
        }

        memcpy(msg + out, remaining, split);
        out += split;
        remaining += split;

        msg[out++] = '-';
        msg[out++] = '\n';
    }

    msg[out] = '\0';

    if (resultingHeight)
        *resultingHeight = MeasureTextEx(font, msg, font_size, spacing).y;

    free(temp);
    return msg;
}

void renderMessages(RenderMessage* gameMessages, int currentIndex, int clientID, int storedMessageCount) {
	float yStart = 470;
	float xStart = 550 + UI_X_OFFSET;
	int messageCount = storedMessageCount;

//RLAPI void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);                       // Draw a line (using triangles/quads)
	float squareSize = GetScreenHeight()/10.0f;



#define FONT_SIZE (20)

	//DrawText("hehehehehehehehehe", xStart-UI_X_OFFSET, squareSize*4, FONT_SIZE, RED);
//fitMessage(const char* message, int font_size, float maxWidth, float* resultingHeight) {
	float currentHeight = 0.0f;
	size_t tempMsgCount = 0;
	RenderMessage* tempMessages = malloc(sizeof(RenderMessage) * messageCount);

	BeginScissorMode(xStart+3, squareSize, (squareSize*4), squareSize*7);
	for (int i = 0; i < messageCount; ++i) {
		int index = (currentIndex + 255 - i) % 256;

		float tempHeight = 0.0f;
		char* msg = fitMessage(gameMessages[index].message, FONT_SIZE, squareSize * 3.1, &tempHeight);

		if (msg) {
			snprintf(tempMessages[tempMsgCount].message, sizeof(tempMessages[tempMsgCount].message), "%s", msg);

			tempMessages[tempMsgCount].id = gameMessages[index].id;
			currentHeight += tempHeight;

			if (tempMessages[tempMsgCount].id == clientID) {
				Font font = GetFontDefault();
				float spacing = FONT_SIZE / 10.0f;
				Vector2 msgDims = MeasureTextEx(font, tempMessages[tempMsgCount].message, FONT_SIZE, spacing);
				Vector2 position = { xStart + squareSize * 4 - 5 - msgDims.x, yStart - currentHeight };
				// DrawRectangleLinesEx((Rectangle){ position.x, position.y, msgDims.x, msgDims.y }, 2.5f, DARKGRAY);
				DrawTextEx(font, tempMessages[tempMsgCount].message, position, FONT_SIZE, spacing, GREEN);
			}
			else
				DrawText(tempMessages[tempMsgCount].message, xStart + 10, yStart - currentHeight, FONT_SIZE, BLACK);

			tempMsgCount++;
			free(msg);
		}
	}
	EndScissorMode();
	DrawRectangleLinesEx(
			(Rectangle){
				xStart+3, squareSize,
				(squareSize*4), squareSize*7
			}, 2.f, GRAY
			);
	free(tempMessages);
}
