#include "../include/helpers.h"
#include <stdint.h>

ssize_t recvAll(int socketfd, void* buffer, size_t expectedSize, int flags) {
	size_t currentRecievedSize = 0;

	while(currentRecievedSize < expectedSize) {
		ssize_t res = recv(socketfd, (uint8_t*)buffer + (currentRecievedSize), expectedSize - currentRecievedSize, flags);
		if (res <= 0) return res;

		currentRecievedSize += res;
	}

	return currentRecievedSize;
}

ssize_t sendAll(int socketfd, const void* buffer, size_t expectedSize, int flags) {
	size_t currentSentSize = 0;

	while(currentSentSize < expectedSize) {
		ssize_t res = send(socketfd, (uint8_t*)buffer + (currentSentSize), expectedSize - currentSentSize, flags);
		if (res <= 0) return res;

		currentSentSize += res;
	}

	return currentSentSize;
}
