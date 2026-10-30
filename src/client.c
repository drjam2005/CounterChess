#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>

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
atomic_bool queueExit = false;
int errno = 0;

// for inputting
char ipBox[256] = "0.0.0.0";
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

pthread_t packetHandler;
pthread_t packetSender;
// queued packets
fromServerPacketQueue currentServerPackets = {};
pthread_mutex_t currentServerPacketQueue = PTHREAD_MUTEX_INITIALIZER;

fromClientPacketQueue toServerPacketsQueue = {};
pthread_mutex_t toServerPacketsQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t toServerPacketsQueueReady = PTHREAD_COND_INITIALIZER;

void stopPacketSender(void) {
	pthread_mutex_lock(&toServerPacketsQueueMutex);
	queueExit = true;
	pthread_cond_broadcast(&toServerPacketsQueueReady);
	pthread_mutex_unlock(&toServerPacketsQueueMutex);
}

void queuePacket(const fromClientPacket packet) {
	pthread_mutex_lock(&toServerPacketsQueueMutex);
	if (enqueueClientPacketQueue(&toServerPacketsQueue, packet) != 0)
		return_error("outgoing packet queue is full");
	pthread_mutex_unlock(&toServerPacketsQueueMutex);
	pthread_cond_signal(&toServerPacketsQueueReady);
}

void* sendQueuedPackets(void* sFD) {
	int serverFD = *(int*)sFD;
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
				popClientPacketQueue(&toServerPacketsQueue) != 0)
				return_error("failed to read outgoing packet queue");
			pthread_mutex_unlock(&toServerPacketsQueueMutex);
		}

		if(sendAll(serverFD, &packet, sizeof packet, 0) <= 0)
			return_error("sendAll error...");
	}
}

void* handleServerPackets(void* sFD) {
	int serverFD = *(int*)sFD;
	while(true) {
		fromServerPacket serverPacket;
		ssize_t res = recvAll(serverFD, &serverPacket, sizeof serverPacket, 0);

		// error handling
		if(res <= 0) return NULL;
		if(serverPacket.packetType == PACKET_SERVER_FULL) {
			stopPacketSender();
			return_error("ERROR: %s", serverPacket.message);
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
			if (enqueueServerPacketQueue(&currentServerPackets, serverPacket) == -1)
				return_error("incoming packet queue is full");
			pthread_mutex_unlock(&currentServerPacketQueue);
		}

	}
}

bool handleGameState(GameState* gameState, fromServerPacketQueue* currentServerPackets);
void updateBoard(GameState* gameState);

void render_checkmate_screen(enum PIECE_COLOR clr);

int handleConnection(const char* port, const char* addr);

int main(int argc, char** argv) {
	int serverSocket;
	bool argumentsProvided = false;
	if(argc == 3)  {
		serverSocket = handleConnection(argv[2], argv[1]);
		argumentsProvided = true;
	}


	memset(&gameState, 0, sizeof gameState);
	memset(messageBox, 0, sizeof messageBox);
	strcpy(messageBox, "Message...");

	// initialize rendering
	SetTraceLogLevel(LOG_NONE);
	InitWindow(800, 600, "Client Window");
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
						serverSocket = handleConnection(portBox, ipBox);
						if (serverSocket == -1) {
							portBox[0] = '\0';
							ipBox[0] = '\0';
							continue;
						} else
							argumentsProvided = true;
					}

				}

			EndDrawing();
			continue;
		}

		// game loop
		if(IsKeyPressed(KEY_ESCAPE) || WindowShouldClose()) {
			stopPacketSender();
		}

		{
			pthread_mutex_lock(&currentServerPacketQueue);
			pthread_mutex_lock(&gameStateMutex);
				handleGameState(&gameState, &currentServerPackets);
			pthread_mutex_unlock(&currentServerPacketQueue);
			pthread_mutex_unlock(&gameStateMutex);
		}

		updateBoard(&gameState);

		BeginDrawing();
		ClearBackground(WHITE);
		DrawText(TextFormat("ID: %d", clientID), 10, 20, FONT_SIZE, BLACK);

		if (gameState.currentColorTurn == myColor) {
			DrawText("YOUR TURN", 80, 20, FONT_SIZE, GREEN);
		} else {
			DrawText("NOT YOUR TURN", 80, 20, FONT_SIZE, RED);
		}

		// main board
		RenderInfo info = {
			.chosenStart = chosenStart,
			.playerColor = myColor
		};

		if (gameState.gameStarted) 
			renderBoard(gameState, info);
		else
			DrawText("Waiting for Players...", 100, 100, 20, BLACK);

		if(gameState.isCheckmate) {
			render_checkmate_screen(gameState.checkedColor);
		}

		// Message stuff
		if (GuiTextBox(
            (Rectangle){ 560, 500, 200, 40 },
            messageBox,
            sizeof(messageBox),
            messageEditMode
        )) {
			messageEditMode = !messageEditMode;
		};

        if (GuiButton((Rectangle){ 560, 550, 70, 40 }, "send")) {
			// queue message
			if(strlen(messageBox)) {
				fromClientPacket packet = {};
				packet.packetType = PACKET_SEND_MESSAGE;
				strcpy(packet.message, messageBox);

				queuePacket(packet);
			}

			messageBox[0] = '\0';
        }

		// render all messages
		float yStart = 460;
		float xStart = 550;
		int messageCount = currentIndex;

		for (int i = 0; i < 10 && i < messageCount; ++i) {
			int index = messageCount - 1 - i;

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
	shutdown(serverSocket, SHUT_RDWR);
	CloseWindow();
	close(serverSocket);

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

void updateBoard(GameState* gameState) {
	if (gameState->currentColorTurn != myColor)
		return;

    if (!gameState->gameStarted)
        return;

	if (gameState->isCheckmate)
		return;

	if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		return;

    float squareSize = GetScreenHeight() / 10.0f;
    Vector2 mouse = GetMousePosition();

    int x = (int)((mouse.x - squareSize) / squareSize);
    int y = (int)((mouse.y - squareSize) / squareSize);

    if (x < 0 || x >= 8 || y < 0 || y >= 8)
        return;

    if (myColor == COLOR_BLACK) {
        x = 7 - x; y = 7 - y;
    } else if (myColor != COLOR_WHITE) {
        return;
    }

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

		// queue sending packet
		fromClientPacket packet;
		packet.packetType = PACKET_SEND_MOVE;
		packet.chosenStart = chosenStart;
		packet.chosenEnd = chosenEnd;
		queuePacket(packet);

		// temporary update own board for smooth client-side rendering
		ChessPiece tempPiece = gameState->currentChessBoard.pieces[chosenStart.y][chosenStart.x];
		if(tempPiece.type != PIECE_NONE) {
			gameState->currentChessBoard.pieces[chosenEnd.y][chosenEnd.x] = tempPiece;
			gameState->currentChessBoard.pieces[chosenStart.y][chosenStart.x].type = PIECE_NONE;
		}

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
		}
		if(serverPacket.packetType == PACKET_SEND_MESSAGE) {
			RenderMessage msg;
			strcpy(msg.message, serverPacket.message);
			msg.id = serverPacket.messageFromID;
			if (currentIndex < 256) {
				gameMessages[currentIndex++] = msg;
			} else {
				memmove(gameMessages, gameMessages + 1, sizeof gameMessages[0] * 255);
				gameMessages[255] = msg;
			}
		}

		// handle server responses
		if(serverPacket.packetType == PACKET_INFORM_PLAYER_TURN)
			gameState->currentPlayerIDTurn = serverPacket.providedPlayerID;
		if(serverPacket.packetType == PACKET_INFORM_PLAYER_NOT_TURN)
			printf("Not your turn... dummy...\n");
 	}

	return true;
}

int handleConnection(const char* port, const char* addr) {
	int serverSocket, port_no;
	struct sockaddr_in server_addr;
	struct hostent *server;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket < 0) return_error("socket error");

	server = gethostbyname(addr);
	if(server == NULL) return_error("gethostbyname error");

	port_no = atoi(port);
	bzero((char*)&server_addr, sizeof server_addr);
	server_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);

	server_addr.sin_port = htons(port_no);

	int res = connect(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(res == -1) return_error("bind error");

	pthread_create(&packetHandler, NULL, handleServerPackets, &serverSocket);
	pthread_detach(packetHandler);
	pthread_create(&packetSender, NULL, sendQueuedPackets, &serverSocket);
	pthread_detach(packetSender);

	return serverSocket;
}
