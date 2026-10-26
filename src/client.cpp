#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <thread>
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "../include/game_state.h"
#include "../include/packets.h"
#include "../include/helpers.h"
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
std::mutex gameStateMutex;
int clientID;
PIECE_COLOR myColor;
char messageBox[256];
bool messageEditMode = true;
std::atomic<bool> queueExit{false};

// for inputting
char ipBox[256] = "0.0.0.0";
bool ipEditMode = true;
char portBox[256] = "5050";
bool portEditMode = true;

struct RenderMessage {
	int id;
	std::string message;
};

std::vector<RenderMessage> gameMessages;

// queued packets
std::vector<fromServerPacket> recievedServerPackets;
std::deque<fromServerPacket> currentServerPackets;
std::mutex currentServerPacketQueue;

std::deque<fromClientPacket> toServerPacketsQueue;
std::mutex toServerPacketsQueueMutex;
std::condition_variable toServerPacketsQueueReady;

void queuePacket(const fromClientPacket& packet) {
	toServerPacketsQueueMutex.lock();
	toServerPacketsQueue.push_back(packet);
	toServerPacketsQueueMutex.unlock();
	toServerPacketsQueueReady.notify_one();
}

void sendQueuedPackets(int serverFD) {
	while(true) {
		fromClientPacket packet;
		{
			std::unique_lock<std::mutex> lock(toServerPacketsQueueMutex);
			toServerPacketsQueueReady.wait(lock, [] {
				return queueExit || !toServerPacketsQueue.empty();
			});
			if (queueExit) return;
			packet = toServerPacketsQueue.front();
			toServerPacketsQueue.pop_front();
		}

		if(sendAll(serverFD, &packet, sizeof packet, 0) <= 0)
			return_error("sendAll error...");
	}
}

void handleServerPackets(int serverFD) {
	while(true) {
		fromServerPacket serverPacket;
		ssize_t res = recvAll(serverFD, &serverPacket, sizeof serverPacket, 0);

		// error handling
		if(res <= 0) return;
		if(serverPacket.packetType == PACKET_SERVER_FULL) {
			queueExit = true;
			toServerPacketsQueueReady.notify_all();
			return_error("ERROR: %s", serverPacket.message);
			return;
		}
		if(serverPacket.packetType == PACKET_SERVER_SHUTDOWN) {
			queueExit = true;
			toServerPacketsQueueReady.notify_all();
			return;
		}

		{ // misc handling
		}

		currentServerPacketQueue.lock();
			currentServerPackets.push_back(serverPacket);
			recievedServerPackets.push_back(serverPacket);
		currentServerPacketQueue.unlock();

	}
}

bool handleGameState(GameState& gameState, std::deque<fromServerPacket>& currentServerPackets);
void updateBoard(const GameState& gameState);

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
			queueExit = true;
			toServerPacketsQueueReady.notify_all();
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
			queueExit = true;
			toServerPacketsQueueReady.notify_all();
		}

		currentServerPacketQueue.lock();
			gameStateMutex.lock();
			handleGameState(gameState, currentServerPackets);
			gameStateMutex.unlock();
		currentServerPacketQueue.unlock();

		updateBoard(gameState);

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
		int messageCount = gameMessages.size();

		for (int i = 0; i < 10 && i < messageCount; ++i) {
			int index = messageCount - 1 - i;

			int messageWidth = MeasureText(gameMessages[index].message.c_str(), 20);

			if (gameMessages[index].id == clientID) {
				DrawText(gameMessages[index].message.c_str(),
						 xStart + 200 - messageWidth, yStart - (i * 25), 20, GREEN);
			} else {
				DrawText(gameMessages[index].message.c_str(),
						 xStart, yStart - (i * 25), 20, BLACK);
			}
		}
		EndDrawing();
	}
	queueExit = true;
	toServerPacketsQueueReady.notify_all();
	shutdown(serverSocket, SHUT_RDWR);
	CloseWindow();
	close(serverSocket);

	return 0;
}

void updateBoard(const GameState& gameState) {
	if (gameState.currentColorTurn != myColor)
		return;

    if (!gameState.gameStarted)
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
		chosenStart = {x, y};
		chosenEnd = {-1, -1};
		// printf("start: %d %d\n", x, y);
	} else {
		if(chosenStart.x == x && chosenStart.y == y) {
			chosenStart = {-1, -1};
			return;
		}

		chosenEnd = {x, y};

		// queue sending packet
		fromClientPacket packet;
		packet.packetType = PACKET_SEND_MOVE;
		packet.chosenStart = chosenStart;
		packet.chosenEnd = chosenEnd;
		queuePacket(packet);

		// reset start
		chosenStart = {-1, -1};
		//printf("end: %d %d\n", x, y);
	}

}

bool handleGameState(GameState& gameState, std::deque<fromServerPacket>& currentServerPackets) {
	// handle game loop
	// PACKET HANDLING
	for(auto& serverPacket : currentServerPackets) {
		// handle each queued server packet
		// gameState stuff
		if(serverPacket.packetType == PACKET_SEND_ID) {
			clientID = serverPacket.providedPlayerID;
			myColor = (PIECE_COLOR)serverPacket.yourColor;
		}
		if(serverPacket.packetType == PACKET_SEND_GAME_STATE) {
			gameState = serverPacket.gameState;
		}
		if(serverPacket.packetType == PACKET_SEND_MESSAGE) {
			RenderMessage msg;
			msg.message = serverPacket.message;
			msg.id = serverPacket.messageFromID;
			gameMessages.push_back(msg);
		}

		// handle server responses
		if(serverPacket.packetType == PACKET_INFORM_PLAYER_TURN)
			gameState.currentPlayerIDTurn = serverPacket.providedPlayerID;
		if(serverPacket.packetType == PACKET_INFORM_PLAYER_NOT_TURN)
			printf("Not your turn... dummy...\n");
 	}

	// empty queue
	currentServerPackets.clear();
	return true;
}

int handleConnection(const char* port, const char* addr) {
	int serverSocket, port_no;
	struct sockaddr_in server_addr;
	struct hostent *server;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket < 0) return_error("socket error");

	server = gethostbyname(addr);
	if(server == nullptr) return_error("gethostbyname error");

	port_no = atoi(port);
	bzero((char*)&server_addr, sizeof server_addr);
	server_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);

	server_addr.sin_port = htons(port_no);

	int res = connect(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(res == -1) return_error("bind error");

	std::thread packetHandler(handleServerPackets, serverSocket);
	packetHandler.detach();
	std::thread packetSender(sendQueuedPackets, serverSocket);
	packetSender.detach();

	return serverSocket;
}
