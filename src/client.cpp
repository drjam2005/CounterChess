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

#include "../include/game_state.h"
#include "../include/packets.h"
#include "../include/helpers.h"
#include "../vendor/raylib/include/raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "../vendor/raylib/include/raygui.h"

#define return_error(...) \
	do {\
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
		exit(1); \
	} while(0)

// global variables
// game state
GameState gameState;
std::mutex gameStateMutex;
int clientID;

// queued packets
std::vector<fromServerPacket> recievedServerPackets;
std::deque<fromServerPacket> currentServerPackets;
std::mutex currentServerPacketQueue;

void handleServerPackets(int serverFD) {
	while(true) {
		fromServerPacket serverPacket;
		ssize_t res = recvAll(serverFD, &serverPacket, sizeof serverPacket, 0);

		// error handling
		if(res == 0)
			return_error("server died unexpectedly...");
		if(serverPacket.packetType == PACKET_SERVER_FULL)
			return_error("ERROR: %s", serverPacket.message);

		{ // misc handling
		}


		currentServerPacketQueue.lock();
			currentServerPackets.push_back(serverPacket);
			recievedServerPackets.push_back(serverPacket);
		currentServerPacketQueue.unlock();
	}
}

bool handleGameState(GameState& gameState, std::deque<fromServerPacket>& currentServerPackets);

int main(int argc, char** argv) {
	if(argc != 3) return_error("Usage: ./client <ip_address> <port_number>");
	int serverSocket, port_no;
	struct sockaddr_in server_addr;
	struct hostent *server;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket < 0) return_error("socket error");

	server = gethostbyname(argv[1]);
	if(server == nullptr) return_error("gethostbyname error");

	port_no = atoi(argv[2]);
	bzero((char*)&server_addr, sizeof server_addr);
	server_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);

	server_addr.sin_port = htons(port_no);

	int res = connect(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(res == -1) return_error("bind error");

	std::thread packetHandler(handleServerPackets, serverSocket);
	packetHandler.detach();

	memset(&gameState, 0, sizeof gameState);


	// initialize rendering
	SetTraceLogLevel(LOG_NONE);
	InitWindow(300, 400, "Client Window");
	SetTargetFPS(60);

	// start rendering
	while(!WindowShouldClose()) {
		// game loop
		currentServerPacketQueue.lock();
			handleGameState(gameState, currentServerPackets);
		currentServerPacketQueue.unlock();

		BeginDrawing();
		ClearBackground(WHITE);
		DrawText(TextFormat("My ID: %d", clientID), 20, 20, 15, BLACK);

		if (GuiButton((Rectangle){ 20, 100, 50, 40 }, "Send 1")) {
			fromClientPacket myPacket;
			myPacket.packetType = PACKET_SEND_MOVE;
			myPacket.chosenMove = 1;
			int res = sendAll(serverSocket, &myPacket, sizeof myPacket, 0);
			if (res <= 0) return_error("sendAll error...");
        }

		if (GuiButton((Rectangle){ 20, 150, 51, 40 }, "Send 2")) {
			fromClientPacket myPacket;
			myPacket.packetType = PACKET_SEND_MOVE;
			myPacket.chosenMove = 2;
			int res = sendAll(serverSocket, &myPacket, sizeof myPacket, 0);
			if (res <= 0) return_error("sendAll error...");
        }

		EndDrawing();
	}
	CloseWindow();
	close(serverSocket);

	return 0;
}

bool handleGameState(GameState& gameState, std::deque<fromServerPacket>& currentServerPackets) {
	// handle game loop
	// PACKET HANDLING
	for(auto& serverPacket : currentServerPackets) {
		// handle each queued server packet
		if(serverPacket.packetType == PACKET_SEND_ID)
			clientID = serverPacket.ID;
 	}

	// empty queue
	currentServerPackets.clear();
	return true;
}
