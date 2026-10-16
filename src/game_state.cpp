#include "../include/game_state.h"
#include <stdio.h>

bool handleGameState(GameState& gameState, std::deque<fromClientPacket>& currentPacketQueue) {
	// if(gameState.player1FD == -1 || gameState.player2FD == -1)
	// 	return true;

	// handle game loop
	int i = 0;
	for(auto& packet : currentPacketQueue) {
		// handle each packet
		if(packet.packetType == PACKET_SEND_MOVE) {
			printf("%d. packet (%d) SEND_MOVE: %d\n", ++i, packet.clientFD, packet.chosenMove);
		}
	}

	// empty queue
	currentPacketQueue.clear();
	return true;
}
