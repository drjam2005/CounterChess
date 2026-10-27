#include "../include/my_structs.h"
#include <string.h>

struct fromClientPacketQueue createClientPacketQueue(void) {
	struct fromClientPacketQueue queue = {0};
	return queue;
}

struct fromServerPacketQueue createServerPacketQueue(void) {
	struct fromServerPacketQueue queue = {0};
	return queue;
}

int enqueueClientPacketQueue(struct fromClientPacketQueue* queue, struct fromClientPacket packet) {
	if (queue == NULL || queue->size >= MAX_QUEUE_SIZE)
		return -1;

	size_t tail = (queue->front + queue->size) % MAX_QUEUE_SIZE;
	queue->data[tail] = packet;
	queue->size++;
	return 0;
}

int enqueueServerPacketQueue(struct fromServerPacketQueue* queue, struct fromServerPacket packet) {
	if (queue == NULL || queue->size >= MAX_QUEUE_SIZE)
		return -1;

	size_t tail = (queue->front + queue->size) % MAX_QUEUE_SIZE;
	queue->data[tail] = packet;
	queue->size++;
	return 0;
}

int frontClientPacketQueue(const struct fromClientPacketQueue* queue, struct fromClientPacket* packetOut) {
	if (queue == NULL || packetOut == NULL || queue->size == 0)
		return -1;

	*packetOut = queue->data[queue->front];
	return 0;
}

int frontServerPacketQueue(const struct fromServerPacketQueue* queue, struct fromServerPacket* packetOut) {
	if (queue == NULL || packetOut == NULL || queue->size == 0)
		return -1;

	*packetOut = queue->data[queue->front];
	return 0;
}

int popClientPacketQueue(struct fromClientPacketQueue* queue) {
	if (queue == NULL || queue->size == 0)
		return -1;

	queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
	queue->size--;
	return 0;
}

int popServerPacketQueue(struct fromServerPacketQueue* queue) {
	if (queue == NULL || queue->size == 0)
		return -1;

	queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
	queue->size--;
	return 0;
}

void clearClientPacketQueue(struct fromClientPacketQueue* queue) {
	if (queue != NULL)
		memset(queue, 0, sizeof *queue);
}

void clearServerPacketQueue(struct fromServerPacketQueue* queue) {
	if (queue != NULL)
		memset(queue, 0, sizeof *queue);
}
