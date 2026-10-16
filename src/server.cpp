#include <deque>
#include <netinet/in.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdlib>
#include <thread>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>

#include "../include/game_state.h"
#include "../include/packets.h"
#include "../vendor/raylib/include/raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "../vendor/raylib/include/raygui.h"

// global variables
std::unordered_map<int, int> FDtoID;
std::vector<ClientMessage> currentMessages;

// queued packets
std::vector<fromClientPacket> recievedClientPackets;
std::deque<fromClientPacket> currentPacketQueue;
std::mutex clientPacketQueueMutex;

std::vector<fromServerPacket> toSendClientPackets;
std::deque<fromServerPacket> toSendPacketQueue;
std::mutex toSendPacketQueueMutex;

// game state
GameState gameState;
std::mutex gameStateMutex;

#define return_error(...) \
	do {\
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
		exit(1); \
	} while(0)

void processSingleClient(int clientFD, int clientID) {
    while(true) {
        char buffer[256] = {};
		fromClientPacket packet;
        ssize_t res = recv(clientFD, &packet, sizeof(packet), 0);

        if(res == 0) {
            printf("Client %d disconnected.\n", clientID);
			gameStateMutex.lock();
				if(gameState.player1FD == clientFD)
					gameState.player1FD = -1;
				else if (gameState.player2FD == clientFD)
					gameState.player2FD = -1;
			gameStateMutex.unlock();
            break;
        }

		{
			clientPacketQueueMutex.lock();
			packet.clientFD = clientFD;
			currentPacketQueue.push_back(packet);
			recievedClientPackets.push_back(packet);
			clientPacketQueueMutex.unlock();
		}

        if(res < 0) return_error("recv error on ClientFD %d", clientFD);
    }

    close(clientFD);
}

void acceptClients(int serverSocket) {
	while(true) {
		int clientFD = accept(serverSocket, 0, 0);
		if(clientFD == -1) return_error("accept error");
		
		// this assumes only 2 clients at most
		gameStateMutex.lock();
		if (gameState.player1FD == -1) {
			gameState.player1FD = clientFD;
			int ID = 1;
			send(clientFD, &ID, sizeof ID, 0);
		} else if (gameState.player2FD == -1) {
			gameState.player2FD = clientFD;
			int ID = 2;
			send(clientFD, &ID, sizeof ID, 0);
		} else {
			close(clientFD);
		}
		gameStateMutex.unlock();

		printf("Accepting client with FD of %d and ID of %d\n", clientFD, FDtoID[clientFD]);

		// send to client it's own ID

		std::thread singleClientThread(processSingleClient, clientFD, FDtoID[clientFD]);
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
	InitWindow(800, 600, "Server");
	SetTargetFPS(60);

	// listen on port
	listen(serverSocket, 5);
	std::thread clientAcceptor(acceptClients, serverSocket);
	clientAcceptor.detach();

	// start rendering
	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(RAYWHITE);

		DrawText("This is server\n", 20, 20, 15, BLACK);

		// game loop
		clientPacketQueueMutex.lock();
			handleGameState(gameState, currentPacketQueue);
		clientPacketQueueMutex.unlock();

		int y = 0;
		int i = 0;
		for(auto& packet : recievedClientPackets){
			DrawText(TextFormat("%d. packet (ClientFD: %d) SEND_MOVE: %d\n", ++i, packet.clientFD, packet.chosenMove),
					20,
					40+y,
					15,
					BLACK
				);
			y+=20;
		}

		EndDrawing();
	}
	CloseWindow();
	close(serverSocket);

	return 0;
}
