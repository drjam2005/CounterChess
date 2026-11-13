#pragma once
#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include "packets.h"
#include "my_structs.h"
#include <pthread.h>
#include <stdatomic.h>

typedef struct queueServerStruct {
	fromServerPacketQueue* toClientQueue;
	pthread_mutex_t* toClientPacketQueueMutex;
	pthread_cond_t* toClientPacketQueueReady;
	atomic_bool* queueExit;
} queueServerStruct;

typedef struct queueClientStruct {
	fromClientPacketQueue* toServerQueue;
	pthread_mutex_t* toServerPacketQueueMutex;
	pthread_cond_t* toServerPacketQueueReady;
	atomic_bool* queueExit;
} queueClientStruct;

bool queueServerPacket(const fromServerPacket packet, queueServerStruct stru);
void queueServerPacketAll(const fromServerPacket packet, const GameState state, queueServerStruct stru);
bool queueClientPacket(const fromClientPacket packet, queueClientStruct stru);
void queueGameState(const GameState state, queueServerStruct stru);

#endif
