#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H

#include "packets.h"
#include <stddef.h>

#define MAX_QUEUE_SIZE 256

struct fromClientPacketQueue {
	struct fromClientPacket data[MAX_QUEUE_SIZE];
	size_t size;
	size_t front;
};

struct fromServerPacketQueue {
	struct fromServerPacket data[MAX_QUEUE_SIZE];
	size_t size;
	size_t front;
};

#ifdef __cplusplus
extern "C" {
#endif

struct fromClientPacketQueue createClientPacketQueue(void);
struct fromServerPacketQueue createServerPacketQueue(void);

// Return 0 on success and -1 if the queue is null, full, or empty as applicable.
int enqueueClientPacketQueue(struct fromClientPacketQueue* queue, struct fromClientPacket packet);
int enqueueServerPacketQueue(struct fromServerPacketQueue* queue, struct fromServerPacket packet);
int frontClientPacketQueue(const struct fromClientPacketQueue* queue, struct fromClientPacket* packetOut);
int frontServerPacketQueue(const struct fromServerPacketQueue* queue, struct fromServerPacket* packetOut);
int popClientPacketQueue(struct fromClientPacketQueue* queue);
int popServerPacketQueue(struct fromServerPacketQueue* queue);

void clearClientPacketQueue(struct fromClientPacketQueue* queue);
void clearServerPacketQueue(struct fromServerPacketQueue* queue);

#ifdef __cplusplus
}
#endif

#endif // MY_STRUCTS_H
