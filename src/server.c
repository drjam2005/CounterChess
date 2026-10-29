#include <pthread.h>
#include <stdatomic.h>
#include <bits/pthreadtypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "../include/game_state.h"
#include "../include/packets.h"
#include "../include/helpers.h"
#include "../include/my_structs.h"
#include "../vendor/raylib/include/raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "../vendor/raylib/include/raygui.h"

#define FONT_SIZE 20

#define return_error(...) \
	do {\
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr);\
		if(errno != 0) perror("ERROR");\
		exit(1); \
	} while(0)

void swap_turn(GameState* state) {
	if(state->currentColorTurn == COLOR_WHITE)
		state->currentColorTurn = COLOR_BLACK;
	else
		state->currentColorTurn = COLOR_WHITE;
}

typedef struct Client {
	int id;
	int fd;
} Client;

// global variables
// game state
GameState gameState;
pthread_mutex_t gameStateMutex = PTHREAD_MUTEX_INITIALIZER;
atomic_bool queueExit = false;
bool broadcastGameState = false;
int errno = 0;

// threads
pthread_t outboundSender;
pthread_t clientAcceptor;

// queued packets
fromClientPacketQueue currentPacketQueue = {};
pthread_mutex_t clientPacketQueueMutex = PTHREAD_MUTEX_INITIALIZER;
fromClientPacket recievedClientPackets[256];
size_t recievedClientPacketCount = 0;

fromServerPacketQueue toClientPacketQueue = {};
pthread_mutex_t toClientPacketQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t toClientPacketQueueReady = PTHREAD_COND_INITIALIZER;

void stopOutboundSender(void) {
	pthread_mutex_lock(&toClientPacketQueueMutex);
	queueExit = true;
	pthread_cond_broadcast(&toClientPacketQueueReady);
	pthread_mutex_unlock(&toClientPacketQueueMutex);
}

void queuePacket(const fromServerPacket packet) {
	pthread_mutex_lock(&toClientPacketQueueMutex);
	if (enqueueServerPacketQueue(&toClientPacketQueue, packet) != 0)
		return_error("outgoing packet queue is full");
	pthread_mutex_unlock(&toClientPacketQueueMutex);
	pthread_cond_signal(&toClientPacketQueueReady);
}

void queuePacketAll(const fromServerPacket packet, const GameState state) {
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


void *sendQueuedPackets(void* d) {
	while (true) {
		fromServerPacket packet;
		pthread_mutex_lock(&toClientPacketQueueMutex);
		while (toClientPacketQueue.size == 0 && !queueExit)
			pthread_cond_wait(&toClientPacketQueueReady, &toClientPacketQueueMutex);

		if (toClientPacketQueue.size == 0 && queueExit) {
			pthread_mutex_unlock(&toClientPacketQueueMutex);
			return NULL;
		}

		if (frontServerPacketQueue(&toClientPacketQueue, &packet) != 0 ||
			popServerPacketQueue(&toClientPacketQueue) != 0) {
			pthread_mutex_unlock(&toClientPacketQueueMutex);
			return_error("failed to read outgoing packet queue");
		}
		pthread_mutex_unlock(&toClientPacketQueueMutex);

		if (sendAll(packet.receivingFD, &packet, sizeof packet, 0) <= 0)
			return_error("sendAll error...");
	}
}

void queueGameState(const GameState state) {
	fromServerPacket packet;
	packet.packetType = PACKET_SEND_GAME_STATE;
	packet.gameState = state;
	queuePacketAll(packet, state);
	broadcastGameState = false;
}

void handleClientDisconnect(int clientFD, int clientID);
bool handleGameState(GameState* gameState, fromClientPacketQueue* currentPacketQueue);

void* handleClientPackets(void* clientArgs) {
	Client client = *(Client*)clientArgs;
	free(clientArgs);
	int clientFD = client.fd;
	int clientID = client.id;

    while(true) {
		if(queueExit)
			break;

		fromClientPacket packet;
        ssize_t res = recvAll(clientFD, &packet, sizeof(packet), 0);

		// error handling
        if (res <= 0) break;

		{ // misc handling
			packet.clientFD = clientFD;
			packet.clientID = clientID;
		}

		pthread_mutex_lock(&clientPacketQueueMutex);
			if (enqueueClientPacketQueue(&currentPacketQueue, packet) != 0)
				return_error("incoming packet queue is full");
			if (recievedClientPacketCount < 256) {
				recievedClientPackets[recievedClientPacketCount++] = packet;
			} else {
				memmove(recievedClientPackets, recievedClientPackets + 1,
					sizeof recievedClientPackets[0] * 255);
				recievedClientPackets[255] = packet;
			}
		pthread_mutex_unlock(&clientPacketQueueMutex);

    }

	handleClientDisconnect(clientFD, clientID);
    close(clientFD);
	return NULL;
}

void *acceptClients(void* serverfd) {
	int serverSocket = *(int*)serverfd;
	while(true) {
		if(queueExit)
			break;
		int clientFD = accept(serverSocket, 0, 0);
		if(clientFD == -1) {
			// Closing the listening socket is how main wakes accept during shutdown.
			if (queueExit)
				return NULL;
			return_error("accept error");
		}
		
		// this assumes only MAX_PLAYERS clients at most
		int id;
		pthread_mutex_lock(&gameStateMutex);
			if (gameState.currentPlayerCount < MAX_PLAYERS) {
				for(id = 1; id <= MAX_PLAYERS; ++id) {
					if (gameState.playerIDtoFD[id] == 0) {
						gameState.playerIDtoFD[id] = clientFD;
						gameState.currentPlayerCount++;
						break;
					}
				}
			} else {
				pthread_mutex_unlock(&gameStateMutex);

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
				gameState.gameStarted = true;
				gameState.currentPlayerIDTurn = 1;
				gameState.currentColorTurn = COLOR_WHITE;
				gameState.currentChessBoard = standardBoard;
				queueGameState(gameState);
			}
			GameState stateSnapshot = gameState;

		pthread_mutex_unlock(&gameStateMutex);

		// send to client it's own ID
		fromServerPacket packet = {};
		packet.packetType = PACKET_SEND_ID;
		packet.providedPlayerID = id;
		packet.yourColor = (int)(id == 1 ? COLOR_WHITE : COLOR_BLACK);
		// save color to state

		gameState.playerIDtoColor[id] = (enum PIECE_COLOR)packet.yourColor;

		packet.receivingFD = clientFD;
		queuePacket(packet);
		queueGameState(stateSnapshot);

		printf("Accepting Client with FD of %d and ID of %d\n", clientFD, id);

		Client* client = malloc(sizeof *client);
		if (client == NULL)
			return_error("failed to allocate client thread arguments");
		client->id = id;
		client->fd = clientFD;
		pthread_t clientThread;
		if (pthread_create(&clientThread, NULL, handleClientPackets, client) != 0) {
			free(client);
			return_error("failed to start client packet reader");
		}
		pthread_detach(clientThread);
	}
	return NULL;
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
	InitWindow(800, 600, "Server");
	SetTargetFPS(60);
	SetExitKey(KEY_NULL);

	memset(&gameState, 0, sizeof gameState);
	gameState.currentPlayerIDTurn = -1;
	pthread_create(&outboundSender, NULL, sendQueuedPackets, NULL);

	// listen on port
	if (listen(serverSocket, 5) == -1) return_error("listen error");
	pthread_create(&clientAcceptor, NULL, acceptClients, &serverSocket);

	// start rendering
	while(!queueExit) {
		// game loop
		if(IsKeyPressed(KEY_ESCAPE) || WindowShouldClose()) {
			GameState stateSnapshot;
			pthread_mutex_lock(&gameStateMutex);
				stateSnapshot = gameState;
			pthread_mutex_unlock(&gameStateMutex);

			fromServerPacket packet = {};
			packet.packetType = PACKET_SERVER_SHUTDOWN;
			queuePacketAll(packet, stateSnapshot);

			stopOutboundSender();
			break;
		}

		// packet handling
		GameState stateSnapshot;
		pthread_mutex_lock(&gameStateMutex);
		pthread_mutex_lock(&clientPacketQueueMutex);
			handleGameState(&gameState, &currentPacketQueue);
			stateSnapshot = gameState;
		pthread_mutex_unlock(&gameStateMutex);
		pthread_mutex_unlock(&clientPacketQueueMutex);

		BeginDrawing();
		ClearBackground(RAYWHITE);

		DrawText("This is server\n", 20, 20, FONT_SIZE, BLACK);
		DrawText(TextFormat("Player Count: %d\n", stateSnapshot.currentPlayerCount), 20, 40, FONT_SIZE, BLACK);
		DrawText(TextFormat("Current turn: %d\n", stateSnapshot.currentPlayerIDTurn), 20, 60, FONT_SIZE, BLACK);

		int y = 0;
		int i = 0;
		{
		pthread_mutex_lock(&clientPacketQueueMutex);
			for (size_t packetIndex = 0; packetIndex < recievedClientPacketCount; ++packetIndex) {
				fromClientPacket* packet = &recievedClientPackets[packetIndex];
				if(packet->packetType == PACKET_SEND_MOVE) {
					DrawText(TextFormat("%d. packet (ClientID: %d) SEND_MOVE\n", ++i, packet->clientID),
							20, 80+y, FONT_SIZE, BLACK);
				}
				if(packet->packetType == PACKET_SEND_MESSAGE) {
					DrawText(TextFormat("%d. packet (ClientID: %d) MESSAGE: %s\n", ++i, packet->clientID, packet->message),
							20, 80+y, FONT_SIZE, BLACK);
				}
				y += FONT_SIZE + 5;
			}
		pthread_mutex_unlock(&clientPacketQueueMutex);
		}

		EndDrawing();
	}

	shutdown(serverSocket, SHUT_RDWR);
	close(serverSocket);
	pthread_join(clientAcceptor, NULL);
	pthread_join(outboundSender, NULL);
	// Drain the shutdown packets before closing client connections.

	// Wake any client reader threads blocked in recv(). They own closing the
	// client sockets, so only issue shutdown here.
	pthread_mutex_lock(&gameStateMutex);
	for (int i = 1; i <= MAX_PLAYERS; ++i) {
		if (gameState.playerIDtoFD[i] != 0)
			shutdown(gameState.playerIDtoFD[i], SHUT_RDWR);
	}
	pthread_mutex_unlock(&gameStateMutex);
	CloseWindow();

	return 0;
}

void handleClientDisconnect(int clientFD, int clientID) {
	printf("Client %d disconnected.\n", clientID);
	pthread_mutex_lock(&gameStateMutex);
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
	pthread_mutex_unlock(&gameStateMutex);
} 

bool handleGameState(GameState* gameState, fromClientPacketQueue* currentPacketQueue) {
	// handle game loop
	// PACKET HANDLING
	broadcastGameState = false;
	while (currentPacketQueue->size > 0) {
		fromClientPacket packet;
		if (frontClientPacketQueue(currentPacketQueue, &packet) != 0)
			return false;
		if (popClientPacketQueue(currentPacketQueue) != 0)
			return false;

		// handle each queued client packet
		if(packet.packetType == PACKET_SEND_MOVE) {
			if (gameState->currentColorTurn != gameState->playerIDtoColor[packet.clientID]) {
				fromServerPacket serverPacket = {};
				serverPacket.packetType = PACKET_INFORM_PLAYER_NOT_TURN;
				serverPacket.receivingFD = packet.clientFD;
				queuePacket(serverPacket);

				printf("Client (%d): SEND_MOVE (%d %d) -> (%d %d) (NOT THEIR TURN)\n", packet.clientID, packet.chosenStart.x, packet.chosenStart.y, packet.chosenEnd.x, packet.chosenEnd.y);
			} else {
				printf("Client (%d): SEND_MOVE (%d %d) -> (%d %d) (THEIR TURN)\n", packet.clientID, packet.chosenStart.x, packet.chosenStart.y, packet.chosenEnd.x, packet.chosenEnd.y);
				if (getPieceAt(gameState->currentChessBoard, packet.chosenStart).color != gameState->playerIDtoColor[packet.clientID]) {
					printf("MOVE INVALID!!!\n");
				} else {
					enum MoveResult res = movePiece(&gameState->currentChessBoard, packet.chosenStart, packet.chosenEnd);
					if ( res < MOVE_VALID ) {
						printf("MOVE INVALID!!!\n");
					} else {
						printf("MOVE VALID!!!\n");
						bool isWhiteChecked = isKingChecked(gameState->currentChessBoard, COLOR_WHITE);
						bool isBlackChecked = isKingChecked(gameState->currentChessBoard, COLOR_BLACK);

						if(isWhiteChecked)
							gameState->checkedColor = COLOR_WHITE;
						else if(isBlackChecked)
							gameState->checkedColor = COLOR_BLACK;
						else
							gameState->checkedColor = COLOR_NONE;

						swap_turn(gameState);
						queueGameState(*gameState);
					}
				}
			}
		}
		if(packet.packetType == PACKET_SEND_MESSAGE) {
			fromServerPacket serverPacket = {};
			serverPacket.packetType = PACKET_SEND_MESSAGE;
			serverPacket.messageFromID = packet.clientID;
			strcpy(serverPacket.message, packet.message);
			queuePacketAll(serverPacket, *gameState);
		}
	}

	// update game state
	// send all players the current game state
	if (broadcastGameState) {
		printf("gameState has been updated, broadcasting...\n");
		queueGameState(*gameState);
	}

	return true;
}
