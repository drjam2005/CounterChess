#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>

#include "../include/game_state.h"
#include "../include/packets.h"
#include "../include/helpers.h"
#include "../include/my_structs.h"
#include "../vendor/raylib/include/raylib.h"
#define RAYGUI_CUSTOM_ICONS
#define RAYGUI_ICON_MAX_FONT_BACKED 257
#define RAYGUI_ICON_FONT_ATLAS_PADDING 1
#include "../include/chess/chess_piece_iconset.h"
#define RAYGUI_IMPLEMENTATION
#include "../vendor/raylib/include/raygui.h"
#include "../include/render_assets.h"

#define FONT_SIZE 25
#define UI_X_OFFSET 200

#define return_error(...) \
	do {\
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
		if(errno != 0) perror("ERROR");\
		exit(1); \
	} while(0)

// global variables
// game state
GameState gameState;
PiecePosition chosenStart = {-1, -1};
PiecePosition chosenEnd = {-1, -1};
pthread_mutex_t gameStateMutex = PTHREAD_MUTEX_INITIALIZER;
int clientID;
enum PIECE_COLOR myColor;
char messageBox[256];
bool messageEditMode = true;
bool isPromoting = false;
bool promotionOpenedThisFrame = false;
atomic_bool queueExit = false;
enum PIECE_TYPE chosenType = PIECE_NONE;

// for inputting
char ipBox[256] = "127.0.0.1";
bool ipEditMode = true;
char portBox[256] = "5050";
bool portEditMode = true;

typedef struct RenderMessage {
	int id;
	char message[256];
} RenderMessage;

// std::vector<RenderMessage> gameMessages;
RenderMessage gameMessages[256];
int currentIndex = 0;
int storedMessageCount = 0;

pthread_t packetHandler;
pthread_t packetSender;
// queued packets
fromServerPacketQueue currentServerPackets = {0};
pthread_mutex_t currentServerPacketQueue = PTHREAD_MUTEX_INITIALIZER;

fromClientPacketQueue toServerPacketsQueue = {0};
pthread_mutex_t toServerPacketsQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t toServerPacketsQueueReady = PTHREAD_COND_INITIALIZER;

void stopPacketSender(void) {
	pthread_mutex_lock(&toServerPacketsQueueMutex);
	queueExit = true;
	pthread_cond_broadcast(&toServerPacketsQueueReady);
	pthread_mutex_unlock(&toServerPacketsQueueMutex);
}

bool queuePacket(const fromClientPacket packet) {
	pthread_mutex_lock(&toServerPacketsQueueMutex);
	if (queueExit) {
		pthread_mutex_unlock(&toServerPacketsQueueMutex);
		return false;
	}
	if (enqueueClientPacketQueue(&toServerPacketsQueue, packet) != 0) {
		pthread_mutex_unlock(&toServerPacketsQueueMutex);
		fprintf(stderr, "outgoing packet queue is full\n");
		return false;
	}
	pthread_cond_signal(&toServerPacketsQueueReady);
	pthread_mutex_unlock(&toServerPacketsQueueMutex);
	return true;
}

void* sendQueuedPackets(void* sFD) {
	int serverFD = *(int*)sFD;
	int sendFlags = 0;
#ifdef MSG_NOSIGNAL
	sendFlags = MSG_NOSIGNAL;
#endif
	while(true) {
		fromClientPacket packet;
		{
			pthread_mutex_lock(&toServerPacketsQueueMutex);
			while (toServerPacketsQueue.size == 0 && !queueExit)
				pthread_cond_wait(&toServerPacketsQueueReady, &toServerPacketsQueueMutex);
			if (queueExit) {
				pthread_mutex_unlock(&toServerPacketsQueueMutex);
				return NULL;
			}
			if (frontClientPacketQueue(&toServerPacketsQueue, &packet) != 0 ||
				popClientPacketQueue(&toServerPacketsQueue) != 0) {
				pthread_mutex_unlock(&toServerPacketsQueueMutex);
				fprintf(stderr, "failed to read outgoing packet queue\n");
				stopPacketSender();
				return NULL;
			}
			pthread_mutex_unlock(&toServerPacketsQueueMutex);
		}

		if(sendAll(serverFD, &packet, sizeof packet, sendFlags) <= 0) {
			if (!queueExit) perror("sendAll error");
			stopPacketSender();
			return NULL;
		}
	}
}

void* handleServerPackets(void* sFD) {
	int serverFD = *(int*)sFD;
	while(true) {
		fromServerPacket serverPacket;
		ssize_t res = recvAll(serverFD, &serverPacket, sizeof serverPacket, 0);

		// error handling
		if(res != (ssize_t)sizeof serverPacket) {
			if (res < 0 && !queueExit) perror("recvAll error");
			stopPacketSender();
			return NULL;
		}
		if(serverPacket.packetType == PACKET_SERVER_FULL) {
			fprintf(stderr, "ERROR: %.*s\n", (int)sizeof serverPacket.message, serverPacket.message);
			stopPacketSender();
			return NULL;
		}
		if(serverPacket.packetType == PACKET_SERVER_SHUTDOWN) {
			stopPacketSender();
			return NULL;
		}

		{ // misc handling
		}

		{
			//std::lock_guard<std::mutex> lock(currentServerPacketQueue);
			pthread_mutex_lock(&currentServerPacketQueue);
			if (enqueueServerPacketQueue(&currentServerPackets, serverPacket) == -1) {
				pthread_mutex_unlock(&currentServerPacketQueue);
				fprintf(stderr, "incoming packet queue is full\n");
				stopPacketSender();
				return NULL;
			}
			pthread_mutex_unlock(&currentServerPacketQueue);
		}

	}
}

bool handleGameState(GameState* gameState, fromServerPacketQueue* currentServerPackets);
void updateBoard(GameState* gameState);

void render_checkmate_screen(enum PIECE_COLOR clr);
void render_stalemate_screen();
void get_promotion_piece(PiecePosition position, enum PIECE_COLOR clr);

int handleConnection(const char* port, const char* addr, int* stableSocketFD);

int main(int argc, char** argv) {
	int serverSocket = -1;
	bool argumentsProvided = false;
	if(argc == 3)  {
		argumentsProvided = handleConnection(argv[2], argv[1], &serverSocket) >= 0;
	}


	memset(&gameState, 0, sizeof gameState);
	memset(messageBox, 0, sizeof messageBox);
	strcpy(messageBox, "Message...");

	// initialize rendering
	SetTraceLogLevel(LOG_NONE);
	InitWindow(1000, 600, "Client Window");
	loadPieceTextures();
	SetTargetFPS(60);

	// start rendering
	while(!queueExit) {
		if(IsKeyPressed(KEY_ESCAPE) || WindowShouldClose()) {
			stopPacketSender();
		}
		if(!argumentsProvided) {
			BeginDrawing();
			ClearBackground(RAYWHITE);
				DrawText("Connect To Server", 100, 100, 30, BLACK);

				if (GuiTextBox(
					(Rectangle){ 100, 140, 400, 50 },
					ipBox,
					sizeof(ipBox),
					ipEditMode
				)) {
					ipEditMode = !ipEditMode;
				};

				if (GuiTextBox(
					(Rectangle){ 100, 200, 400, 50 },
					portBox,
					sizeof(portBox),
					portEditMode
				)) {
					portEditMode = !portEditMode;
				};

				if (GuiButton((Rectangle){ 100, 260, 70, 40 }, "connect")) {
					// queue message
					if(strlen(portBox) && strlen(ipBox)) {
						if (handleConnection(portBox, ipBox, &serverSocket) == -1) {
							fprintf(stderr, "Connection failed; check the address and port.\n");
						} else
							argumentsProvided = true;
					}

				}

			EndDrawing();
			continue;
		}

		// game loop
		promotionOpenedThisFrame = false;

		bool packetsHandled;
		{
			pthread_mutex_lock(&currentServerPacketQueue);
			pthread_mutex_lock(&gameStateMutex);
				packetsHandled = handleGameState(&gameState, &currentServerPackets);
			pthread_mutex_unlock(&gameStateMutex);
			pthread_mutex_unlock(&currentServerPacketQueue);
		}
		if (!packetsHandled) {
			fprintf(stderr, "failed to process incoming packet queue\n");
			stopPacketSender();
		}

		updateBoard(&gameState);

		BeginDrawing();
		ClearBackground(WHITE);
		//DrawText(TextFormat("ID: %d", clientID), 10, 20, FONT_SIZE, BLACK);

		//if (gameState.currentColorTurn == myColor) {
		//	DrawText("YOUR TURN", 80, 20, FONT_SIZE, GREEN);
		//} else {
		//	DrawText("NOT YOUR TURN", 80, 20, FONT_SIZE, RED);
		//}

		// main board
		struct RenderInfo info = {
			.chosenStart = chosenStart,
			.playerColor = myColor,
			.isPromoting = isPromoting
		};

		if (gameState.gameStarted) 
			renderBoard(gameState, info);
		else
			DrawText("Waiting for Players...", 100 + UI_X_OFFSET, 100, 20, BLACK);

		if(gameState.isCheckmate) {
			render_checkmate_screen(gameState.checkedColor);
		}
		if(gameState.isStalemate) {
			render_stalemate_screen();
		}

		if(isPromoting) {
			get_promotion_piece(chosenEnd, myColor);
		}

		// Message stuff
		if (GuiTextBox(
            (Rectangle){ 560 + UI_X_OFFSET, 500, 200, 40 },
            messageBox,
            sizeof(messageBox),
            messageEditMode
        )) {
			messageEditMode = !messageEditMode;
		};

        if (GuiButton((Rectangle){ 560 + UI_X_OFFSET, 550, 70, 40 }, "send")) {
			// queue message
			if(strlen(messageBox)) {
				fromClientPacket packet = {0};
				packet.packetType = PACKET_SEND_MESSAGE;
				snprintf(packet.message, sizeof packet.message, "%s", messageBox);

				if (queuePacket(packet))
					messageBox[0] = '\0';
			}
        }

		// render all messages
		float yStart = 460;
		float xStart = 550 + UI_X_OFFSET;
		int messageCount = storedMessageCount;

		for (int i = 0; i < 10 && i < messageCount; ++i) {
			int index = (currentIndex + 255 - i) % 256;

			int messageWidth = MeasureText(gameMessages[index].message, 20);

			if (gameMessages[index].id == clientID) {
				DrawText(gameMessages[index].message,
						 xStart + 200 - messageWidth, yStart - (i * 25), 20, GREEN);
			} else {
				DrawText(gameMessages[index].message,
						 xStart, yStart - (i * 25), 20, BLACK);
			}
		}
		EndDrawing();
	}
	stopPacketSender();
	if (serverSocket >= 0) {
		shutdown(serverSocket, SHUT_RDWR);
		pthread_join(packetHandler, NULL);
		pthread_join(packetSender, NULL);
		close(serverSocket);
	}
	unloadPieceTextures();
	CloseWindow();

	return 0;
}

void render_checkmate_screen(enum PIECE_COLOR clr) {
	if(clr == COLOR_WHITE) {
		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GetColor(0xFFFFFF55));
		DrawText("WHITE IS CHECKMATED\nBLACK WINS!\n", 100, 100, 40, BLACK);
	} else if (clr == COLOR_BLACK) {
		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GetColor(0xFFFFFF55));
		DrawText("BLACK IS CHECKMATED\nWHITE WINS!\n", 100, 100, 40, BLACK);
	}
	return;
}

void render_stalemate_screen() {

	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GetColor(0xFFFFFF55));
	DrawText("STALEMATE!!! DRAW!!!\n", 100, 100, 40, BLACK);

	return;
}

static void submit_promotion(enum PIECE_TYPE pieceType) {
	fromClientPacket packet = {0};
	packet.packetType = PACKET_SEND_MOVE;
	packet.chosenStart = chosenStart;
	packet.chosenEnd = chosenEnd;
	packet.chosenType = pieceType;
	if (!queuePacket(packet))
		return;

	isPromoting = false;
	promotionOpenedThisFrame = false;
	chosenStart = (PiecePosition){-1, -1};
	chosenEnd = (PiecePosition){-1, -1};
	chosenType = PIECE_NONE;
}

void get_promotion_piece(PiecePosition position, enum PIECE_COLOR clr) {
	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GetColor(0xFFFFFF55));

	PiecePosition screenPos = toBoardPosition(position.x, position.y, clr);
	float squareSize = GetScreenHeight()/10.0f;
	int pixelSize = squareSize/15;
	int direction = screenPos.y == 7 ? -1 : 1;
	enum PIECE_TYPE options[] = { PIECE_QUEEN, PIECE_KNIGHT, PIECE_ROOK, PIECE_BISHOP };
	Rectangle optionRects[4];
	for (int i = 0; i < 4; ++i) {
		float x = (screenPos.x + 1) * squareSize + UI_X_OFFSET;
		float y = (screenPos.y + direction * i + 1) * squareSize;
		optionRects[i] = (Rectangle){ x, y, squareSize, squareSize };
		DrawRectangleRec(optionRects[i], RED);
		GuiDrawIcon(options[i] + 256, (int)x, (int)y, pixelSize, clr == COLOR_WHITE ? WHITE : BLACK);
	}

	// The destination click that opens this menu must not also choose an option.
	if(!promotionOpenedThisFrame && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		Vector2 mousePos = GetMousePosition();
		for (int i = 0; i < 4; ++i) {
			if (CheckCollisionPointRec(mousePos, optionRects[i])) {
				chosenType = options[i];
				submit_promotion(chosenType);
				break;
			}
		}
	}

	return;
}

void updateBoard(GameState* gameState) {
	if (gameState->currentColorTurn != myColor)
		return;

    if (!gameState->gameStarted)
        return;

	if (gameState->isCheckmate || gameState->isStalemate)
		return;

	if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		return;

    float squareSize = GetScreenHeight() / 10.0f;
    Vector2 mouse = GetMousePosition();

    int x = (int)((mouse.x - squareSize - UI_X_OFFSET) / squareSize);
    int y = (int)((mouse.y - squareSize) / squareSize);

    if (x < 0 || x >= 8 || y < 0 || y >= 8)
        return;

    if (myColor == COLOR_BLACK) {
        x = 7 - x; y = 7 - y;
    } else if (myColor != COLOR_WHITE) {
        return;
    }

	if(isPromoting)
		return;

	if (chosenStart.x == -1 && chosenStart.y == -1) {
		chosenStart = (PiecePosition){x, y};
		chosenEnd = (PiecePosition){-1, -1};
		// printf("start: %d %d\n", x, y);
	} else {
		if(chosenStart.x == x && chosenStart.y == y) {
			chosenStart = (PiecePosition){-1, -1};
			return;
		}

		chosenEnd = (PiecePosition){x, y};

		// TODO:
		//	   check if chosen piece is PAWN and chosenEnd is promotion for such
		//	   pawn, then we should alert the client to have the user choose their
		//	   piece
		if (getPieceAt(gameState->currentChessBoard, chosenStart).type == PIECE_PAWN) {
			enum MoveResult result = evaluateMove(&gameState->currentChessBoard, chosenStart, chosenEnd);
			if(result & MOVE_PROMOTION) {
				isPromoting = true;
				promotionOpenedThisFrame = true;
				return;
			}
		}

		// queue sending packet
		fromClientPacket packet = {0};
		packet.packetType = PACKET_SEND_MOVE;
		packet.chosenStart = chosenStart;
		packet.chosenEnd = chosenEnd;
		packet.chosenType = PIECE_NONE;
		if (!queuePacket(packet))
			return;

		// temporary update own board for smooth client-side rendering
//		ChessPiece tempPiece = gameState->currentChessBoard.pieces[chosenStart.y][chosenStart.x];
//		if(tempPiece.type != PIECE_NONE) {
//			gameState->currentChessBoard.pieces[chosenEnd.y][chosenEnd.x] = tempPiece;
//			gameState->currentChessBoard.pieces[chosenStart.y][chosenStart.x].type = PIECE_NONE;
//		}

		// reset start
		chosenStart = (PiecePosition){-1, -1};
		//printf("end: %d %d\n", x, y);
	}

}

bool handleGameState(GameState* gameState, fromServerPacketQueue* currentServerPackets) {
	// handle game loop
	// PACKET HANDLING
	while (currentServerPackets->size > 0) {
		fromServerPacket serverPacket;
		if (frontServerPacketQueue(currentServerPackets, &serverPacket) != 0)
			return false;
		if (popServerPacketQueue(currentServerPackets) != 0)
			return false;

		// handle each queued server packet
		// gameState stuff
		if(serverPacket.packetType == PACKET_SEND_ID) {
			clientID = serverPacket.providedPlayerID;
			myColor = (enum PIECE_COLOR)serverPacket.yourColor;
		}
		if(serverPacket.packetType == PACKET_SEND_GAME_STATE) {
			*gameState = serverPacket.gameState;

			// gameState might also change colors
			myColor = gameState->playerIDtoColor[clientID];
		}
		if(serverPacket.packetType == PACKET_SEND_MESSAGE) {
			RenderMessage msg;
			snprintf(msg.message, sizeof msg.message, "%.*s",
				(int)sizeof serverPacket.message, serverPacket.message);
			msg.id = serverPacket.messageFromID;
			gameMessages[currentIndex] = msg;
			currentIndex = (currentIndex + 1) % 256;
			if (storedMessageCount < 256)
				storedMessageCount++;
		}

		// handle server responses
		if(serverPacket.packetType == PACKET_INFORM_PLAYER_TURN)
			gameState->currentPlayerIDTurn = serverPacket.providedPlayerID;
		if(serverPacket.packetType == PACKET_INFORM_PLAYER_NOT_TURN)
			printf("Not your turn... dummy...\n");
		if(serverPacket.packetType == PACKET_INFORM_PLAYER_SNAPSHOT)
			printf("snapshot: %s\n", serverPacket.snapshotMessage);
 	}

	return true;
}

int handleConnection(const char* port, const char* addr, int* stableSocketFD) {
	int serverSocket = -1;
	struct addrinfo hints = {0};
	struct addrinfo *serverAddresses = NULL;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;

	int res = getaddrinfo(addr, port, &hints, &serverAddresses);
	if (res != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(res));
		return -1;
	}

	int lastSocketError = 0;
	for (struct addrinfo* server = serverAddresses; server != NULL; server = server->ai_next) {
		serverSocket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
		if (serverSocket < 0) {
			lastSocketError = errno;
			continue;
		}

		res = connect(serverSocket, server->ai_addr, server->ai_addrlen);
		if (res == 0)
			break;

		lastSocketError = errno;
		close(serverSocket);
		serverSocket = -1;
	}
	freeaddrinfo(serverAddresses);

	if (serverSocket < 0) {
		errno = lastSocketError;
		perror("connect error");
		return -1;
	}

	// Main owns stableSocketFD until both packet threads have finished.
	*stableSocketFD = serverSocket;
	res = pthread_create(&packetHandler, NULL, handleServerPackets, stableSocketFD);
	if (res != 0) {
		fprintf(stderr, "pthread_create packetHandler: %s\n", strerror(res));
		close(serverSocket);
		*stableSocketFD = -1;
		return -1;
	}
	res = pthread_create(&packetSender, NULL, sendQueuedPackets, stableSocketFD);
	if (res != 0) {
		fprintf(stderr, "pthread_create packetSender: %s\n", strerror(res));
		stopPacketSender();
		shutdown(serverSocket, SHUT_RDWR);
		pthread_join(packetHandler, NULL);
		close(serverSocket);
		*stableSocketFD = -1;
		queueExit = false;
		return -1;
	}

	return serverSocket;
}
