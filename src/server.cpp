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
#define RAYGUI_IMPLEMENTATION
#include "../vendor/raylib/include/raygui.h"

#define FONT_SIZE 25

#define return_error(...) \
	do {\
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr);\
		if(errno != 0) perror("ERROR");\
		exit(1); \
	} while(0)

// global variables
// game state
GameState gameState;
std::mutex gameStateMutex;
std::atomic<bool> queueExit{false};
bool broadcastGameState = false;

// queued packets
std::vector<fromClientPacket> recievedClientPackets;
std::deque<fromClientPacket> currentPacketQueue;
std::mutex clientPacketQueueMutex;

std::deque<fromServerPacket> toClientPacketQueue;
std::mutex toClientPacketQueueMutex;
std::condition_variable toClientPacketQueueReady;

void queuePacket(const fromServerPacket& packet) {
	toClientPacketQueueMutex.lock();
	toClientPacketQueue.push_back(packet);
	toClientPacketQueueMutex.unlock();
	toClientPacketQueueReady.notify_one();
}

void queuePacketAll(const fromServerPacket& packet, const GameState& state) {
	fromServerPacket tempPacket = packet;
	int clientFDs[MAX_PLAYERS];

	for (int i = 0; i < MAX_PLAYERS; ++i)
		clientFDs[i] = state.playerIDtoFD[i + 1];

	for (int i = 0; i < MAX_PLAYERS; ++i) {
		int clientFD = clientFDs[i];
		if (clientFD == 0)
			continue;
		tempPacket.receivingFD = clientFD;
		queuePacket(tempPacket);
	}
}


void sendQueuedPackets() {
	while(true) {
		fromServerPacket packet;
		{
			std::unique_lock<std::mutex> lock(toClientPacketQueueMutex);
			toClientPacketQueueReady.wait(lock, [] {
				return queueExit || !toClientPacketQueue.empty();
			});
			if (toClientPacketQueue.empty() && queueExit) return;
			packet = toClientPacketQueue.front();
			toClientPacketQueue.pop_front();
		}

		if(sendAll(packet.receivingFD, &packet, sizeof packet, 0) <= 0)
			return_error("sendAll error...");
	}
}

void queueGameState(const GameState& state) {
	for (int i = 1; i <= MAX_PLAYERS; ++i) {
		if (state.playerIDtoFD[i] == 0) continue;
		fromServerPacket packet = {};
		packet.packetType = PACKET_SEND_GAME_STATE;
		packet.gameState = state;
		packet.receivingFD = state.playerIDtoFD[i];
		queuePacket(packet);
	}

	broadcastGameState = false;
}

void handleClientDisconnect(int clientFD, int clientID);
bool handleGameState(GameState& gameState, std::deque<fromClientPacket>& currentPacketQueue);

void handleClientPackets(int clientFD, int clientID) {
    while(true) {
		if(queueExit)
			break;

        char buffer[256] = {};
		fromClientPacket packet;
        ssize_t res = recvAll(clientFD, &packet, sizeof(packet), 0);

		// error handling
        if (res <= 0) break;

		{ // misc handling
			packet.clientFD = clientFD;
			packet.clientID = clientID;
		}

		clientPacketQueueMutex.lock();
			currentPacketQueue.push_back(packet);
			recievedClientPackets.push_back(packet);
		clientPacketQueueMutex.unlock();

    }

	handleClientDisconnect(clientFD, clientID);
    close(clientFD);
}

void acceptClients(int serverSocket) {
	while(true) {
		if(queueExit)
			break;
		int clientFD = accept(serverSocket, 0, 0);
		if(clientFD == -1) {
			if (queueExit) return;
			return_error("accept error");
		}
		
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

				fromServerPacket packet = {};
				packet.packetType = PACKET_SERVER_FULL;
				packet.receivingFD = clientFD;
				packet.providedPlayerID = -1;
				snprintf(packet.message, sizeof packet.message, "Server is full (max of %d players)", MAX_PLAYERS);

				if (sendAll(clientFD, &packet, sizeof packet, 0) <= 0)
					fprintf(stderr, "failed to send server-full response\n");
				close(clientFD);
				continue;
			}

			// check if amount of players is sufficient
			if(gameState.currentPlayerCount == MAX_PLAYERS) {
				// game start
				gameState.currentPlayerIDTurn = 1;
			}
			GameState stateSnapshot = gameState;

		gameStateMutex.unlock();

		// send to client it's own ID
		fromServerPacket packet = {};
		packet.packetType = PACKET_SEND_ID;
		packet.providedPlayerID = id;
		packet.receivingFD = clientFD;
		queuePacket(packet);
		queueGameState(stateSnapshot);

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
	SetExitKey(KEY_NULL);

	memset(&gameState, 0, sizeof gameState);
	gameState.currentPlayerIDTurn = -1;
	std::thread outboundSender(sendQueuedPackets);

	// listen on port
	listen(serverSocket, 5);
	std::thread clientAcceptor(acceptClients, serverSocket);
	clientAcceptor.detach();

	// start rendering
	while(!queueExit) {
		// game loop
		if(IsKeyPressed(KEY_ESCAPE) || WindowShouldClose()) {
			GameState stateSnapshot;
			gameStateMutex.lock();
				stateSnapshot = gameState;
			gameStateMutex.unlock();

			for (int i = 1; i <= MAX_PLAYERS; ++i) {
				if (stateSnapshot.playerIDtoFD[i] == 0) continue;
				fromServerPacket packet = {};
				packet.packetType = PACKET_SERVER_SHUTDOWN;
				packet.receivingFD = stateSnapshot.playerIDtoFD[i];
				queuePacket(packet);
			}
			queueExit = true;
			toClientPacketQueueReady.notify_all();
			break;
		}

		clientPacketQueueMutex.lock();
			gameStateMutex.lock();
			handleGameState(gameState, currentPacketQueue);
			GameState stateSnapshot = gameState;
			gameStateMutex.unlock();
		clientPacketQueueMutex.unlock();

		BeginDrawing();
		ClearBackground(RAYWHITE);

		DrawText("This is server\n", 20, 20, FONT_SIZE, BLACK);
		DrawText(TextFormat("Player Count: %d\n", stateSnapshot.currentPlayerCount), 20, 40, FONT_SIZE, BLACK);

		int y = 0;
		int i = 0;
		clientPacketQueueMutex.lock();
		for(auto& packet : recievedClientPackets){
			if(packet.packetType == PACKET_SEND_MOVE) {
				DrawText(TextFormat("%d. packet (ClientID: %d) SEND_MOVE: %d\n", ++i, packet.clientID, packet.chosenMove),
						20, 80+y,
						FONT_SIZE, BLACK);
			y+= FONT_SIZE + 5;
			}
		}
		clientPacketQueueMutex.unlock();

		EndDrawing();
	}
	toClientPacketQueueReady.notify_all();
	shutdown(serverSocket, SHUT_RDWR);
	close(serverSocket);
	outboundSender.join();
	CloseWindow();

	return 0;
}

void handleClientDisconnect(int clientFD, int clientID) {
	printf("Client %d disconnected.\n", clientID);
	gameStateMutex.lock();
		for(int i = 1; i <= MAX_PLAYERS; ++i) {
			if(gameState.playerIDtoFD[i] == clientFD) {
				gameState.playerIDtoFD[i] = 0;
				gameState.currentPlayerCount--;
				if (gameState.currentPlayerCount < MAX_PLAYERS)
					gameState.currentPlayerIDTurn = -1;
				break;
			}
		}
		if (!queueExit)
			broadcastGameState = true;
	gameStateMutex.unlock();
} 

bool handleGameState(GameState& gameState, std::deque<fromClientPacket>& currentPacketQueue) {
	// handle game loop
	// PACKET HANDLING
	broadcastGameState = false;
	for(auto& packet : currentPacketQueue) {
		// handle each queued client packet
		if(packet.packetType == PACKET_SEND_MOVE) {
			printf("Client (%d): SEND_MOVE %d\n", packet.clientID, packet.chosenMove);

			broadcastGameState = true;
		}
		if(packet.packetType == PACKET_SEND_MESSAGE) {
			fromServerPacket serverPacket = {};
			serverPacket.packetType = PACKET_SEND_MESSAGE;
			snprintf(serverPacket.message, sizeof serverPacket.message,"Client %d: %s\n", packet.clientID, packet.message);
			queuePacketAll(serverPacket, gameState);
		}
	}

	if (broadcastGameState) {
		printf("gameState has been updated, broadcasting...\n");
		queueGameState(gameState);
	}

	// empty queue
	currentPacketQueue.clear();
	return true;
}
