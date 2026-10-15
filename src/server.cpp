#include <netinet/in.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdlib>
#include <cstring>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../include/packets.h"

// global variables

std::unordered_map<int, int> FDtoID;
std::vector<ClientMessage> currentMessages;

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
        ssize_t res = recv(clientFD, buffer, sizeof(buffer) - 1, 0);

        if(res == 0) {
            printf("Client %d disconnected.\n", clientID);
			FDtoID[clientFD] = -1;
            break;
        }

        if(res < 0) return_error("recv error on ClientFD %d", clientFD);

        ClientMessage msgPacket{};
		msgPacket.clientFD = clientFD;
        msgPacket.clientID = clientID;

        memcpy(msgPacket.message, buffer, res);
        msgPacket.message[res] = '\0';

        currentMessages.push_back(msgPacket);

        for(auto FtI : FDtoID) {
            // if(FtI.first == clientFD)
            //     continue;
			if(FtI.second == -1) continue; // disconnected client
										   //
            ssize_t sent = send(FtI.first, &msgPacket, sizeof msgPacket, 0);
            if(sent < 0) return_error("send error...");
        }
    }

    close(clientFD);
}

void acceptClients(int serverSocket) {
	while(true) {
		int clientFD = accept(serverSocket, 0, 0);
		if(clientFD == -1) return_error("accept error");
		FDtoID[clientFD] = FDtoID.size() + 1;
		printf("Accepting client with FD of %d and ID of %d\n", clientFD, FDtoID[clientFD]);

		// send to client it's own ID
		int ID = FDtoID[clientFD];
		send(clientFD, &ID, sizeof ID, 0);

		std::thread singleClientThread(processSingleClient, clientFD, FDtoID[clientFD]);
		singleClientThread.detach();
	}
}


int main(int argc, char** argv) {
	if(argc != 2) return_error("Usage: ./server <port_number>");

	int serverSocket, port_no, n;
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

	// listen on port
	listen(serverSocket, 5);
	std::thread clientAcceptor(acceptClients, serverSocket);
	clientAcceptor.detach();

	while(true) { }

	close(serverSocket);
}
