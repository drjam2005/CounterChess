#include <pthread.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/plug/plug.h"
#include "../include/handle_game.h"
#include "../include/game_state.h"
#include "../include/packet_handler.h"
#include "../include/helpers.h"
#include "../include/my_structs.h"
#include "../vendor/raylib/include/raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "../vendor/raylib/include/raygui.h"

#define return_error(...) \
	do {\
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr);\
		if(errno != 0) perror("ERROR");\
		exit(1); \
	} while(0)


TWISTS_DONE randomizeBoard(GameState* state);
void randomizePieces(GameState* state);


typedef struct Client {
	int id;
	int fd;
} Client;

// global variables
// game state
GameState gameState = {0};
pthread_mutex_t gameStateMutex = PTHREAD_MUTEX_INITIALIZER;
atomic_bool queueExit = false;
bool broadcastGameState = false;
PlugServer plug = {0};

// threads
pthread_t outboundSender;
pthread_t clientAcceptor;

// queued packets
fromClientPacketQueue currentPacketQueue = {};
pthread_mutex_t clientPacketQueueMutex = PTHREAD_MUTEX_INITIALIZER;
fromClientPacket recievedClientPackets[256];
size_t recievedClientPacketCount = 0;
size_t recievedClientPacketNextIndex = 0;
unsigned long long recievedClientPacketTotal = 0;

fromServerPacketQueue toClientPacketQueue = {};
pthread_mutex_t toClientPacketQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t toClientPacketQueueReady = PTHREAD_COND_INITIALIZER;

// Track detached client readers until they have fully closed their sockets.
pthread_mutex_t clientReadersMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clientReadersFinished = PTHREAD_COND_INITIALIZER;
size_t activeClientReaders = 0;

queueServerStruct stru;

void stopOutboundSender(void) {
	pthread_mutex_lock(&toClientPacketQueueMutex);
	queueExit = true;
	pthread_cond_broadcast(&toClientPacketQueueReady);
	pthread_mutex_unlock(&toClientPacketQueueMutex);
}

void *sendQueuedPackets(void* d) {
	(void)d;
	int sendFlags = 0;
#ifdef MSG_NOSIGNAL
	sendFlags = MSG_NOSIGNAL;
#endif
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
			fprintf(stderr, "failed to read outgoing packet queue\n");
			stopOutboundSender();
			return NULL;
		}
		pthread_mutex_unlock(&toClientPacketQueueMutex);

		if (sendAll(packet.receivingFD, &packet, sizeof packet, sendFlags) != (ssize_t)sizeof packet) {
			if (!queueExit)
				perror("sendAll error");
			// Do not terminate the entire server when a single client disconnects.
		}
		close(packet.receivingFD);
	}
}

void handleClientDisconnect(int clientFD, int clientID);

void* handleClientPackets(void* clientArgs) {
	Client client = *(Client*)clientArgs;
	free(clientArgs);
	int clientFD = client.fd;
	int clientID = client.id;

    while(true) {
		if(queueExit)
			break;

		fromClientPacket packet = {0};
        ssize_t res = recvAll(clientFD, &packet, sizeof(packet), 0);

		// error handling
        if (res != (ssize_t)sizeof packet) break;

		{ // misc handling
			packet.clientFD = clientFD;
			packet.clientID = clientID;
		}

		pthread_mutex_lock(&clientPacketQueueMutex);
			if (enqueueClientPacketQueue(&currentPacketQueue, packet) != 0) {
				pthread_mutex_unlock(&clientPacketQueueMutex);
				fprintf(stderr, "incoming packet queue is full; dropping client packet\n");
				continue;
			}
			recievedClientPackets[recievedClientPacketNextIndex] = packet;
			recievedClientPacketNextIndex = (recievedClientPacketNextIndex + 1) % 256;
			if (recievedClientPacketCount < 256)
				recievedClientPacketCount++;
			recievedClientPacketTotal++;
		pthread_mutex_unlock(&clientPacketQueueMutex);

    }

	handleClientDisconnect(clientFD, clientID);
    close(clientFD);
	pthread_mutex_lock(&clientReadersMutex);
	if (activeClientReaders > 0)
		activeClientReaders--;
	pthread_cond_broadcast(&clientReadersFinished);
	pthread_mutex_unlock(&clientReadersMutex);
	return NULL;
}

void *acceptClients(void* serverfd) {
	int serverSocket = *(int*)serverfd;
	while(true) {
		if(queueExit)
			break;
		struct sockaddr_in clientAddress;
		socklen_t clientAddressSize = sizeof clientAddress;
		int clientFD = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressSize);
		if(clientFD == -1) {
			// Closing the listening socket is how main wakes accept during shutdown.
			if (queueExit)
				return NULL;
			if (errno == EINTR)
				continue;
			perror("accept error");
			continue;
		}

		if (queueExit) {
			close(clientFD);
			break;
		}

		// Limit a stalled client's ability to block the outgoing packet sender.
		struct timeval sendTimeout = {.tv_sec = 3, .tv_usec = 0};
		setsockopt(clientFD, SOL_SOCKET, SO_SNDTIMEO, &sendTimeout, sizeof sendTimeout);

		// this assumes only 2 MAX_PLAYERS clients at most
		int id = 0;
		pthread_mutex_lock(&gameStateMutex);
			if (gameState.currentPlayerCount < MAX_PLAYERS) {
				for(id = 1; id <= MAX_PLAYERS; ++id) {
					if (gameState.playerIDtoFD[id] == 0) {
						gameState.playerIDtoFD[id] = clientFD;
						gameState.playerIDtoColor[id] = id == 1 ? COLOR_WHITE : COLOR_BLACK;
						gameState.currentPlayerCount++;
						break;
					}
				}
			}

			if (id < 1 || id > MAX_PLAYERS) {
				pthread_mutex_unlock(&gameStateMutex);

				fromServerPacket packet = {0};
				packet.packetType = PACKET_SERVER_FULL;
				packet.receivingFD = clientFD;
				packet.providedPlayerID = -1;
				snprintf(packet.message, sizeof packet.message, "Server is full (max of %d players)", MAX_PLAYERS);

				if (sendAll(clientFD, &packet, sizeof packet, 0) != (ssize_t)sizeof packet)
					fprintf(stderr, "failed to send server-full response\n");
				close(clientFD);
				continue;
			}

			// check if amount of players is sufficient
			if(gameState.currentPlayerCount == MAX_PLAYERS) {
				// game start
				gameState.gameStarted = true;
				gameState.isCheckmate = false;
				gameState.isStalemate = false;
				gameState.isRotated = false;
				gameState.checkedColor = COLOR_NONE;
				gameState.currentPlayerIDTurn = 1;
				gameState.currentColorTurn = COLOR_WHITE;
				gameState.currentChessBoard = standardBoard;
			}
			GameState stateSnapshot = gameState;

		// send to client it's own ID
		fromServerPacket packet = {0};
		packet.packetType = PACKET_SEND_ID;
		packet.providedPlayerID = id;
		packet.yourColor = (int)(id == 1 ? COLOR_WHITE : COLOR_BLACK);
		// save color to state
		// Color was stored under gameStateMutex during player assignment.

		packet.receivingFD = clientFD;
		// Keep the ID and initial snapshot ordered with respect to game updates.
		bool queuedID = queueServerPacket(packet, stru);
		if (queuedID)
			queueGameState(stateSnapshot, stru);
		pthread_mutex_unlock(&gameStateMutex);
		if (!queuedID) {
			handleClientDisconnect(clientFD, id);
			close(clientFD);
			continue;
		}

		printf("Client connected (FD %d, player %d).\n", clientFD, id);

		Client* client = malloc(sizeof *client);
		if (client == NULL) {
			fprintf(stderr, "failed to allocate client thread arguments\n");
			handleClientDisconnect(clientFD, id);
			close(clientFD);
			continue;
		}
		client->id = id;
		client->fd = clientFD;
		pthread_t clientThread;
		pthread_mutex_lock(&clientReadersMutex);
		activeClientReaders++;
		pthread_mutex_unlock(&clientReadersMutex);
		int createResult = pthread_create(&clientThread, NULL, handleClientPackets, client);
		if (createResult != 0) {
			fprintf(stderr, "failed to start client packet reader: %s\n", strerror(createResult));
			free(client);
			handleClientDisconnect(clientFD, id);
			close(clientFD);
			pthread_mutex_lock(&clientReadersMutex);
			activeClientReaders--;
			pthread_cond_broadcast(&clientReadersFinished);
			pthread_mutex_unlock(&clientReadersMutex);
			continue;
		}
		pthread_detach(clientThread);
	}
	return NULL;
}

int main(int argc, char** argv) {
	srand((unsigned)time(NULL));
	if(argc != 2) return_error("Usage: ./server <port_number>");

	char* end = NULL;
	errno = 0;
	long requestedPort = strtol(argv[1], &end, 10);
	if (errno != 0 || *argv[1] == '\0' || *end != '\0' || requestedPort < 1025 || requestedPort > 65535)
		return_error("invalid port (must be 1025-65535)");

	printf("Server starting ...\n");
	int serverSocket, port_no;
	struct sockaddr_in server_addr;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket < 0) return_error("socket error");

	int reuse = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse) != 0)
		perror("setsockopt SO_REUSEADDR");

	// bind to port
	memset(&server_addr, 0, sizeof server_addr);
	port_no = (int)requestedPort;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port_no);

	int res = bind(serverSocket, (struct sockaddr *)&server_addr, sizeof server_addr);
	if (res == -1) return_error("bind error");
	
	stru = (queueServerStruct){
		.toClientPacketQueueMutex = &toClientPacketQueueMutex,
		.toClientPacketQueueReady = &toClientPacketQueueReady,
		.toClientQueue = &toClientPacketQueue,
		.queueExit = &queueExit
	};

	plug.libpath = "./build/libhandle_game.so";
	reloadPlugServer(&plug);

	// initialize rendering
	SetTraceLogLevel(LOG_NONE);
	InitWindow(800, 600, "CounterChess: Server");
	SetTargetFPS(60);
	SetExitKey(KEY_NULL);

	memset(&gameState, 0, sizeof gameState);
	gameState.currentPlayerIDTurn = -1;
	res = pthread_create(&outboundSender, NULL, sendQueuedPackets, NULL);
	if (res != 0) return_error("failed to start outgoing packet thread: %s", strerror(res));

	// listen on port
	if (listen(serverSocket, 5) == -1) return_error("listen error");
	printf("Server listening to port %d ...\n", port_no);
	printf("Waiting for connection(s) ...\n");
	res = pthread_create(&clientAcceptor, NULL, acceptClients, &serverSocket);
	if (res != 0) return_error("failed to start client acceptor: %s", strerror(res));

	// start rendering
	while(!queueExit) {
		// game loop
		if(IsKeyPressed(KEY_ESCAPE) || WindowShouldClose()) {
			GameState stateSnapshot;
			pthread_mutex_lock(&gameStateMutex);
				stateSnapshot = gameState;
				fromServerPacket packet = {0};
				packet.packetType = PACKET_SERVER_SHUTDOWN;
				queueServerPacketAll(packet, stateSnapshot, stru);
			pthread_mutex_unlock(&gameStateMutex);

			stopOutboundSender();
			break;
		}

		if(IsKeyPressed(KEY_R) && IsKeyDown(KEY_LEFT_CONTROL)) {
			reloadPlugServer(&plug);
		}

		// packet handling
		GameState stateSnapshot;
		pthread_mutex_lock(&gameStateMutex);
		pthread_mutex_lock(&clientPacketQueueMutex);
		GameInfo info = { .broadcastGameState = broadcastGameState };
			if (!plug.handleGameState(&gameState, &currentPacketQueue, info, stru))
				fprintf(stderr, "failed to handle incoming game packets\n");
			else
				broadcastGameState = false;
			stateSnapshot = gameState;
		pthread_mutex_unlock(&clientPacketQueueMutex);
		pthread_mutex_unlock(&gameStateMutex);

		BeginDrawing();
		ClearBackground(RAYWHITE);

		int FONT_SIZE = (int)(GetScreenHeight()/30.0f);

		DrawText(TextFormat("Player Count: %d\n", stateSnapshot.currentPlayerCount), 20, FONT_SIZE, FONT_SIZE, BLACK);
		switch(stateSnapshot.currentColorTurn) {
			case COLOR_WHITE:
				DrawText("Current Turn: WHITE", 20, FONT_SIZE+FONT_SIZE, FONT_SIZE, BLACK);
				break;
			case COLOR_BLACK:
				DrawText("Current Turn: BLACK", 20, FONT_SIZE+FONT_SIZE, FONT_SIZE, BLACK);
				break;
			default:
				break;
		}

		fromClientPacket recentPackets[20];
		size_t recentCount;
		unsigned long long recentTotal;
		pthread_mutex_lock(&clientPacketQueueMutex);
			recentCount = recievedClientPacketCount < 20 ? recievedClientPacketCount : 20;
			recentTotal = recievedClientPacketTotal;
			for (size_t packetIndex = 0; packetIndex < recentCount; ++packetIndex) {
				size_t index = (recievedClientPacketNextIndex + 255 - packetIndex) % 256;
				recentPackets[packetIndex] = recievedClientPackets[index];
			}
		pthread_mutex_unlock(&clientPacketQueueMutex);

		DrawText("Client Packet Logs:", 20, FONT_SIZE*3.5f, FONT_SIZE, BLACK);
		DrawRectangle(20, FONT_SIZE*5, GetScreenWidth()-40, FONT_SIZE*20, GRAY);
		for (size_t packetIndex = 0; packetIndex < recentCount; ++packetIndex) {
			fromClientPacket* packet = &recentPackets[packetIndex];
			const char* packet_type = "UNKNOWN PACKET";
			if(packet->packetType == PACKET_SEND_MOVE) {
				packet_type = "PACKET_SEND_MOVE";
			}
			if(packet->packetType == PACKET_SEND_MESSAGE) {
				packet_type = "PACKET_SEND_MESSAGE";
			}
			DrawText(TextFormat("%llu. packet (ClientID: %d) TYPE: %s\n",
				recentTotal - packetIndex, packet->clientID, packet_type),
				20, (FONT_SIZE*23) - (int)(packetIndex * FONT_SIZE), FONT_SIZE, BLACK);
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
	pthread_mutex_lock(&clientReadersMutex);
	while (activeClientReaders > 0)
		pthread_cond_wait(&clientReadersFinished, &clientReadersMutex);
	pthread_mutex_unlock(&clientReadersMutex);
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
				if (gameState.currentPlayerCount < MAX_PLAYERS) {
					gameState.currentPlayerIDTurn = -1;
					gameState.gameStarted = false;
				}
				break;
			}
		}

		// check if all disconnected
		bool stillPlayer = false;
		for(int i = 1; i <= MAX_PLAYERS; ++i) {
			if (gameState.playerIDtoFD[i] != 0) {
				stillPlayer = true;
				break;
			}
		}

		if(!stillPlayer) {
			memset(&gameState, 0, sizeof(GameState));
			gameState.currentPlayerIDTurn = -1;
		}

		if (!queueExit)
			broadcastGameState = true;
	pthread_mutex_unlock(&gameStateMutex);
}

void randomizePieces(GameState* state){
	if (state == NULL) return;

	ChessBoard tempBoard = state->currentChessBoard;

	enum PIECE_TYPE chosenType = PIECE_NONE;
	enum PIECE_TYPE endType    = PIECE_NONE;
	while((chosenType <= PIECE_INVALID || chosenType >= PIECE_COUNT) || chosenType == PIECE_KING)
		chosenType = (enum PIECE_TYPE)(rand() % PIECE_COUNT);

	bool causesCheck = ifCheckmate(tempBoard, COLOR_WHITE) || ifCheckmate(tempBoard, COLOR_BLACK);
	do {
		causesCheck = false;

		while((endType <= PIECE_INVALID || endType >= PIECE_COUNT) || endType == chosenType || endType == PIECE_KING)
			endType = (enum PIECE_TYPE)(rand() % PIECE_COUNT);

		printf("trying %s -> %s\n", TypeToString(chosenType), TypeToString(endType));

		for(int y = 0; y < 8; ++y) {
			for(int x = 0; x < 8; ++x) {
				if (tempBoard.pieces[y][x].type == chosenType && tempBoard.pieces[y][x].color == state->currentColorTurn)
					tempBoard.pieces[y][x].type = endType;
			}
		}
		causesCheck = ifCheckmate(tempBoard, COLOR_WHITE) || ifCheckmate(tempBoard, COLOR_BLACK);

		if(causesCheck)
			printf("Causes check!!! trying something else...\n");

	} while(causesCheck);

	printf("swapped %s -> %s\n", TypeToString(chosenType), TypeToString(endType));


	return;
}


