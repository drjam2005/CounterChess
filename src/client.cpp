#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../vendor/raylib/include/raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "../vendor/raylib/include/raygui.h"
#include "../include/packets.h"

#define return_error(...) \
	do {\
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
		exit(1); \
	} while(0)


#include <deque>
std::deque<fromServerPacket> currentServerPackets;

// globals
int clientID;

void handleIncomingPackets(int serverFD) {
	while(true) {
		fromServerPacket serverPacket;
		int res = recv(serverFD, &serverPacket, sizeof serverPacket, 0);
		if(res == 0) return_error("server closed unexpectedly...");

		if(serverPacket.packetType == PACKET_SERVER_FULL)
			return_error("ERROR: %s", serverPacket.message);
		else if(serverPacket.packetType == PACKET_SEND_ID)
			clientID = serverPacket.ID;

		// if(res != sizeof msgPacket) return_error("TODO: incomplete recieve size");
		currentServerPackets.push_back(serverPacket);
	}
}

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

	std::thread packetHandler(handleIncomingPackets, serverSocket);
	packetHandler.detach();


	// rendering
	SetTraceLogLevel(LOG_NONE);
	InitWindow(300, 400, "Client Window");
	SetTargetFPS(60);

	char messageBuffer[256];
	memset(messageBuffer, 0, sizeof messageBuffer);
	bool editMode = true;
	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(WHITE);
		DrawText(TextFormat("My ID: %d", clientID), 20, 20, 15, BLACK);

		if (GuiButton((Rectangle){ 20, 100, 50, 40 }, "Send 1")) {
			fromClientPacket myPacket;
			myPacket.packetType = PACKET_SEND_MOVE;
			myPacket.chosenMove = 1;
			send(serverSocket, &myPacket, sizeof myPacket, 0);
        }

		if (GuiButton((Rectangle){ 20, 150, 51, 40 }, "Send 2")) {
			fromClientPacket myPacket;
			myPacket.packetType = PACKET_SEND_MOVE;
			myPacket.chosenMove = 2;
			send(serverSocket, &myPacket, sizeof myPacket, 0);
        }

		// PACKET HANDLING
		for(auto& packet : currentServerPackets) {
			// handle each queued server packet
		}

		currentServerPackets.clear();

		EndDrawing();
	}
	CloseWindow();
	return 0;
}
