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


#include <vector>
std::vector<ClientMessage> currentMessages;

void handleIncomingPackets(int serverFD) {
	while(true) {
		ClientMessage msgPacket;
		int res = recv(serverFD, &msgPacket, sizeof msgPacket, 0);
		if(res == 0) return_error("server closed unexpectedly...");

		// if(res != sizeof msgPacket) return_error("TODO: incomplete recieve size");

		currentMessages.push_back(msgPacket);
	}
}

int main(int argc, char** argv) {
	if(argc != 3) return_error("Usage: ./client <ip_address> <port_number>");

	// try connecting
	struct addrinfo hints, *llist, *p;
	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags	  =	AI_PASSIVE;

	// validate port
	int value = atoi(argv[2]);
	if(value <= 1024) return_error("port number must be greater than 1024");

	int res = getaddrinfo(argv[1], argv[2], &hints, &llist);
	if(res != 0) return_error("getaddrinfo error");

	int serverSocket;
	for(p = llist; p != NULL; p = p->ai_next) {
		serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(serverSocket == -1) return_error("socket error");

		int res = connect(serverSocket, p->ai_addr, p->ai_addrlen);
		if(res == -1) return_error("bind error");

		break;
	}

	if(p == NULL) return_error("no valid addresses found");

	int clientID;
	recv(serverSocket, &clientID, sizeof clientID, 0);
	std::thread packetHandler(handleIncomingPackets, serverSocket);
	packetHandler.detach();

	// rendering
	InitWindow(300, 400, "Client Window");
	SetTargetFPS(60);

	char messageBuffer[256];
	memset(messageBuffer, 0, sizeof messageBuffer);
	bool editMode = true;
	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(WHITE);

		// handle sending of messages
		if (GuiTextBox(
			(Rectangle){ 20, 20, 200, 40 },
			messageBuffer,
			sizeof(messageBuffer),
			editMode))
        {
            editMode = !editMode;
        }


		if (GuiButton((Rectangle){ 230, 20, 50, 40 }, "Send")) {
            if (messageBuffer[0] != '\0')
            {
                TraceLog(LOG_INFO, "Message: %s", messageBuffer);
                // Process or send message here.
				
				size_t len = strlen(messageBuffer) + 1;
				ssize_t sent = send(serverSocket, messageBuffer, len, 0);
				if(sent < 0)
					perror("send");
                messageBuffer[0] = '\0'; // Clear input after sending
            }
        }

		DrawText("Only renders the\n last 10 messages", 20, 70, 10, BLACK);

		// Render the last 10 texts, newest first
		int y = 0;

		for (int i = 0; i < 10 && i < currentMessages.size(); ++i) {
			const ClientMessage& msg = currentMessages[currentMessages.size() - 1 - i];

			if (msg.clientID == clientID)
				DrawText(TextFormat("[You] > %s", msg.message), 20, 100 + y, 15, RED);
			else
				DrawText(TextFormat("[Client %d] > %s", msg.clientID, msg.message), 20, 100 + y, 15, BLACK);

			y += 20;
		}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
