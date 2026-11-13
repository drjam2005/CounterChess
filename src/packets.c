#include "../include/packet_handler.h"
#include "../include/my_structs.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

void queueServerPacketAll(const fromServerPacket packet, const GameState state, queueServerStruct stru){
	fromServerPacket tempPacket = packet;
	int clientFDs[MAX_PLAYERS];

	for (int i = 0; i < MAX_PLAYERS; ++i)
		clientFDs[i] = state.playerIDtoFD[i + 1];

	for (int i = 0; i < MAX_PLAYERS; ++i) {
		int clientFD = clientFDs[i];
		if (clientFD == 0)
			continue;
		tempPacket.receivingFD = clientFD;
		queueServerPacket(tempPacket, stru);
	}
}

bool queueServerPacket(const fromServerPacket packet, queueServerStruct stru) {
	pthread_mutex_lock(stru.toClientPacketQueueMutex);
	if (*stru.queueExit) {
		pthread_mutex_unlock(stru.toClientPacketQueueMutex);
		return false;
	}
	// Retain a separate socket descriptor until this packet is sent.
	// This prevents a disconnected client's FD from being reused for an older packet.
	fromServerPacket queuedPacket = packet;
	queuedPacket.receivingFD = dup(packet.receivingFD);
	if (queuedPacket.receivingFD == -1) {
		pthread_mutex_unlock(stru.toClientPacketQueueMutex);
		perror("dup outgoing socket");
		return false;
	}
	if (enqueueServerPacketQueue(stru.toClientQueue, queuedPacket) != 0) {
		close(queuedPacket.receivingFD);
		pthread_mutex_unlock(stru.toClientPacketQueueMutex);
		fprintf(stderr, "outgoing packet queue is full; packet was not sent\n");
		return false;
	}
	pthread_cond_signal(stru.toClientPacketQueueReady);
	pthread_mutex_unlock(stru.toClientPacketQueueMutex);
	return true;
}

bool queueClientPacket(const fromClientPacket packet, queueClientStruct stru) {
	pthread_mutex_lock(stru.toServerPacketQueueMutex);
	if (*stru.queueExit) {
		pthread_mutex_unlock(stru.toServerPacketQueueMutex);
		return false;
	}
	if (enqueueClientPacketQueue(stru.toServerQueue, packet) != 0) {
		pthread_mutex_unlock(stru.toServerPacketQueueMutex);
		fprintf(stderr, "outgoing packet queue is full\n");
		return false;
	}
	pthread_cond_signal(stru.toServerPacketQueueReady);
	pthread_mutex_unlock(stru.toServerPacketQueueMutex);
	return true;
}

void queueGameState(const GameState state, queueServerStruct stru){
	fromServerPacket packet = {0};
	packet.packetType = PACKET_SEND_GAME_STATE;
	packet.gameState = state;
	queueServerPacketAll(packet, state, stru);
}
