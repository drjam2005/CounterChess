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

// queued packets
std::vector<fromClientPacket> recievedClientPackets;
std::deque<fromClientPacket> currentPacketQueue;
std::mutex clientPacketQueueMutex;

std::vector<fromServerPacket> toSendClientPackets;
std::deque<fromServerPacket> toSendPacketQueue;
std::mutex toSendPacketQueueMutex;

void handleClientDisconnect(int clientFD, int clientID);
bool handleGameState(GameState& gameState, std::deque<fromClientPacket>& currentPacketQueue);

void handleClientPackets(int clientFD, int clientID) {
    while(true) {
        char buffer[256] = {};
		fromClientPacket packet;
        ssize_t res = recvAll(clientFD, &packet, sizeof(packet), 0);
		packet.clientFD = clientFD;

		// error handling
        if (res == 0) {
			handleClientDisconnect(clientFD, clientID); break;
        }

		{ // misc handling
		}

		clientPacketQueueMutex.lock();
			currentPacketQueue.push_back(packet);
			recievedClientPackets.push_back(packet);
		clientPacketQueueMutex.unlock();

        if (res < 0) 
			return_error("recv error on ClientFD %d", clientFD);
    }

    close(clientFD);
}

void acceptClients(int serverSocket) {
	while(true) {
		int clientFD = accept(serverSocket, 0, 0);
		if(clientFD == -1) return_error("accept error");
		
		// this assumes only MAX_PLAYERS clients at most
		int id;
		gameStateMutex.lock();
			if (gameState.currentPlayerCount < MAX_PLAYERS) {
				for(id = 1; id <= MAX_PLAYERS; ++id) {
					if (gameState.playerIDtoFD[id] == 0) {
						gameState.playerIDtoFD[id] = clientFD;
						gameState.currentPlayerCount++;
						break;
					}
				}
			} else {
				gameStateMutex.unlock();
				fromServerPacket packet;
				packet.packetType = PACKET_SERVER_FULL;
				snprintf(packet.message, sizeof packet.message, "Server is full (max of %d players)", MAX_PLAYERS);
				packet.ID = -1;
				int res = sendAll(clientFD, &packet, sizeof packet, 0);
				if (res <= 0) return_error("sendAll error...");

				close(clientFD);
				continue;
			}
		gameStateMutex.unlock();

		// send to client it's own ID
		fromServerPacket packet;
		packet.packetType = PACKET_SEND_ID;
		packet.ID = id;
		int res = sendAll(clientFD, &packet, sizeof packet, 0);
		if (res <= 0) return_error("sendAll error...");

		printf("Accepting Client with FD of %d and ID of %d\n", clientFD, id);

		std::thread singleClientThread(handleClientPackets, clientFD, id);
		singleClientThread.detach();
	}
}


int main(int argc, char** argv) {
	if(argc != 2) return_error("Usage: ./server <port_number>");

	int serverSocket, port_no;
	struct sockaddr_in server_addr;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket < 0) return_error("socket error");

	// bind to port
	bzero((char*)&server_addr, sizeof server_addr);
	port_no = atoi(argv[1]);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port_no);

	int res = bind(serverSocket, (struct sockaddr *)&server_addr, sizeof server_addr);
	if (res == -1) return_error("bind error");

	// initialize rendering
	SetTraceLogLevel(LOG_NONE);
	InitWindow(400, 600, "Server");
	SetTargetFPS(60);

	// listen on port
	listen(serverSocket, 5);
	std::thread clientAcceptor(acceptClients, serverSocket);
	clientAcceptor.detach();

	memset(&gameState, 0, sizeof gameState);

	// start rendering
	while(!WindowShouldClose()) {
		// game loop
		clientPacketQueueMutex.lock();
			handleGameState(gameState, currentPacketQueue);
		clientPacketQueueMutex.unlock();

		BeginDrawing();
		ClearBackground(RAYWHITE);

		DrawText("This is server\n", 20, 20, 15, BLACK);
		DrawText(TextFormat("Player Count: %d\n", gameState.currentPlayerCount), 20, 40, 15, BLACK);

		int y = 0;
		int i = 0;
		for(auto& packet : recievedClientPackets){
			DrawText(TextFormat("%d. packet (ClientFD: %d) SEND_MOVE: %d\n", ++i, packet.clientFD, packet.chosenMove),
					20, 60+y,
					15, BLACK);
			y+=20;
		}

		EndDrawing();
	}
	CloseWindow();
	close(serverSocket);

	return 0;
}

void handleClientDisconnect(int clientFD, int clientID) {
	printf("Client %d disconnected.\n", clientID);
	gameStateMutex.lock();
		for(int i = 1; i <= MAX_PLAYERS; ++i) {
			if(gameState.playerIDtoFD[i] == clientFD) {
				gameState.playerIDtoFD[i] = 0;
				gameState.currentPlayerCount--;
				break;
			}
		}
	gameStateMutex.unlock();
} 

bool handleGameState(GameState& gameState, std::deque<fromClientPacket>& currentPacketQueue) {
	// handle game loop
	// PACKET HANDLING
	for(auto& packet : currentPacketQueue) {
		// handle each queued client packet
		if(packet.packetType == PACKET_SEND_MOVE)
			printf("Client %d: PACKET_MOVE\n", packet.clientFD);
	}

	// empty queue
	currentPacketQueue.clear();
	return true;
}
