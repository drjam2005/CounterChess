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

	struct addrinfo hints, *llist, *p;
	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags	  =	AI_PASSIVE;

	// validate port
	int value = atoi(argv[1]);
	if(value <= 1024) return_error("port number must be greater than 1024");

	int res = getaddrinfo(NULL, argv[1], &hints, &llist);
	if(res != 0) return_error("getaddrinfo error");

	int serverSocket;
	for(p = llist; p != NULL; p = p->ai_next) {
		serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(serverSocket == -1) return_error("socket error");

		int res = bind(serverSocket, p->ai_addr, p->ai_addrlen);
		if(res == -1) return_error("bind error");

		break;
	}

	if(p == NULL) return_error("no valid addresses found");

	// listen on port
	listen(serverSocket, 5);
	std::thread clientAcceptor(acceptClients, serverSocket);
	clientAcceptor.detach();

	while(true) { }

	close(serverSocket);
}
