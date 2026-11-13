#include <pthread.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/game_state.h"
#include "../include/handle_game.h"
#include "../include/packet_handler.h"
#include "../include/my_structs.h"

#define return_error(...) \
	do {\
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr);\
		if(errno != 0) perror("ERROR");\
		exit(1); \
	} while(0)

void swap_turn(GameState* state) {
	if(state->currentColorTurn == COLOR_WHITE)
		state->currentColorTurn = COLOR_BLACK;
	else
		state->currentColorTurn = COLOR_WHITE;
}

TWISTS_DONE randomizeBoard(GameState* state, queueServerStruct stru){
	if (state == NULL) return false;

	enum PIECE_TYPE	types[5] = {PIECE_PAWN, PIECE_KNIGHT, PIECE_BISHOP, PIECE_ROOK, PIECE_QUEEN};
	ChessPiece* current_pieces[64];	
	int count_pos = 0;

	for(int x = 0; x < 8; x++){
		for(int y = 0; y < 8; y++){
			ChessPiece* current_piece = &state->currentChessBoard.pieces[x][y];		
			if(current_piece->color == state->currentColorTurn && current_piece->type != PIECE_KING && current_piece->type != PIECE_NONE){
				current_pieces[count_pos++] = current_piece;
			}
		}
	}
	
	if(count_pos == 0) return false;
	
	int chance = rand() % 100;
	
	if(chance < 25){
		int num_of_pieces = rand() % count_pos + 1;

		// Fisher-Yates shuffle (ik ik, its quite a 'random' algorithm to use haha)
		for (int i = count_pos - 1; i > 0; i--) {
			int j = rand() % (i + 1);

			ChessPiece* temp = current_pieces[i];
			current_pieces[i] = current_pieces[j];
			current_pieces[j] = temp;
		}

		for(int i = 0; i < num_of_pieces; i++){
			enum PIECE_TYPE check_type = types[rand() % 5];

			while(current_pieces[i]->type == check_type){
				check_type = types[rand() % 5];
			}

			current_pieces[i]->previousType = current_pieces[i]->type;
			current_pieces[i]->type = check_type;
		}

		fromServerPacket serverPacket = {0};
		serverPacket.packetType = PACKET_INFORM_PLAYER_SNAPSHOT;
		snprintf(serverPacket.snapshotMessage, sizeof serverPacket.snapshotMessage, "TWIST: Some of %s's pieces randomly promoted!", state->currentColorTurn == COLOR_WHITE ? "WHITE" : "BLACK");
		queueServerPacketAll(serverPacket, *state, stru);
		return true;
	}
	
	
	return false;
}

bool handleGameState(GameState* gameState, fromClientPacketQueue* currentPacketQueue, struct GameInfo gameInfo, queueServerStruct stru) {
	// handle game loop
	// PACKET HANDLING
	while (currentPacketQueue->size > 0) {
		fromClientPacket packet;
		char snapshotMessage[256];
		memset(snapshotMessage, 0, sizeof snapshotMessage);

		if (frontClientPacketQueue(currentPacketQueue, &packet) != 0)
			return false;
		if (popClientPacketQueue(currentPacketQueue) != 0)
			return false;

		// handle each queued client packet
		if (packet.clientID < 1 || packet.clientID > MAX_PLAYERS ||
			gameState->playerIDtoFD[packet.clientID] != packet.clientFD)
			continue;

		if(packet.packetType == PACKET_SEND_MOVE) {
			if (!gameState->gameStarted || gameState->currentPlayerCount != MAX_PLAYERS ||
				gameState->isCheckmate || gameState->isStalemate)
				continue;

			if (packet.chosenStart.x < 0 || packet.chosenStart.x >= 8 ||
				packet.chosenStart.y < 0 || packet.chosenStart.y >= 8 ||
				packet.chosenEnd.x < 0 || packet.chosenEnd.x >= 8 ||
				packet.chosenEnd.y < 0 || packet.chosenEnd.y >= 8) {
				fprintf(stderr, "Client (%d): move coordinates out of bounds\n", packet.clientID);
				continue;
			}

			if (gameState->currentColorTurn != gameState->playerIDtoColor[packet.clientID]) {
				fromServerPacket serverPacket = {0};
				serverPacket.packetType = PACKET_INFORM_PLAYER_NOT_TURN;
				serverPacket.receivingFD = packet.clientFD;
				queueServerPacket(serverPacket, stru);

				printf("Client (%d): SEND_MOVE (%d %d) -> (%d %d) (NOT THEIR TURN)\n", packet.clientID, packet.chosenStart.x, packet.chosenStart.y, packet.chosenEnd.x, packet.chosenEnd.y);
			} else {
				printf("Client (%d): SEND_MOVE (%d %d) -> (%d %d) (THEIR TURN)\n", packet.clientID, packet.chosenStart.x, packet.chosenStart.y, packet.chosenEnd.x, packet.chosenEnd.y);
				snprintf(snapshotMessage, sizeof snapshotMessage, "%s (%s): %c%d -> %c%d", (gameState->playerIDtoColor[packet.clientID] == COLOR_WHITE ? "WHITE" : "BLACK"), (TypeToString(getPieceAt(gameState->currentChessBoard, packet.chosenStart).type)), packet.chosenStart.x+'a', 8-packet.chosenStart.y, packet.chosenEnd.x+'a', 8-packet.chosenEnd.y);
				printf("%s\n", snapshotMessage);
				if (getPieceAt(gameState->currentChessBoard, packet.chosenStart).color != gameState->playerIDtoColor[packet.clientID]) {
					printf("MOVE INVALID!!!\n");
				} else {
					// Apply the move to a copy so an invalid move cannot damage the board.
					ChessBoard updatedBoard = gameState->currentChessBoard;
					enum MoveResult res = movePiece(&updatedBoard, packet.chosenStart, packet.chosenEnd, packet.chosenType);
					if (res < MOVE_VALID) {
						printf("MOVE INVALID!!!\n");
					} else {
						gameState->currentChessBoard = updatedBoard;
						printf("MOVE VALID!!!\n");
						// The next player must be evaluated after the successful move.
						enum PIECE_COLOR colorToCheck = gameState->currentColorTurn == COLOR_BLACK ? COLOR_WHITE : COLOR_BLACK;
						bool isWhiteChecked = isKingChecked(gameState->currentChessBoard, COLOR_WHITE);
						bool isBlackChecked = isKingChecked(gameState->currentChessBoard, COLOR_BLACK);

						gameState->isCheckmate = false;
						gameState->isStalemate = false;
						gameState->checkedColor = COLOR_NONE;
						if (isWhiteChecked) {
							gameState->checkedColor = COLOR_WHITE;
							if(ifCheckmate(gameState->currentChessBoard, COLOR_WHITE))
								gameState->isCheckmate = true;
						}
						else if(isBlackChecked) {
							gameState->checkedColor = COLOR_BLACK;
							if(ifCheckmate(gameState->currentChessBoard, COLOR_BLACK))
								gameState->isCheckmate = true;
						} else {
							bool isStaleMate = ifStalemate(gameState->currentChessBoard, colorToCheck);
							gameState->isStalemate = isStaleMate;
							gameState->stalematedColor = colorToCheck;
							if(isStaleMate)
								printf("stalemate!!\n");
						}

						if (gameState->isCheckmate)
							printf("Checkmate!\n");

						if (!gameState->isCheckmate && !gameState->isStalemate) {
							if(randomizeBoard(gameState, stru)) {
								enum PIECE_COLOR colorToCheck = gameState->currentColorTurn == COLOR_BLACK ? COLOR_WHITE : COLOR_BLACK;
								bool isWhiteChecked = isKingChecked(gameState->currentChessBoard, COLOR_WHITE);
								bool isBlackChecked = isKingChecked(gameState->currentChessBoard, COLOR_BLACK);

								gameState->isCheckmate = false;
								gameState->isStalemate = false;
								gameState->checkedColor = COLOR_NONE;
								if (isWhiteChecked) {
									gameState->checkedColor = COLOR_WHITE;
									if(ifCheckmate(gameState->currentChessBoard, COLOR_WHITE))
										gameState->isCheckmate = true;
								}
								else if(isBlackChecked) {
									gameState->checkedColor = COLOR_BLACK;
									if(ifCheckmate(gameState->currentChessBoard, COLOR_BLACK))
										gameState->isCheckmate = true;
								} else {
									bool isStaleMate = ifStalemate(gameState->currentChessBoard, colorToCheck);
									gameState->isStalemate = isStaleMate;
									gameState->stalematedColor = colorToCheck;
									if(isStaleMate)
										printf("stalemate!!\n");
								}
							}
						}

						swap_turn(gameState);
					}
					queueGameState(*gameState, stru);
				}
			}
		}

		if(strlen(snapshotMessage)){
			fromServerPacket serverPacket = {0};
			serverPacket.packetType = PACKET_INFORM_PLAYER_SNAPSHOT;
			strcpy(serverPacket.snapshotMessage, snapshotMessage);
			queueServerPacketAll(serverPacket, *gameState, stru);
		}
		
		if(packet.packetType == PACKET_SEND_MESSAGE) {
			fromServerPacket serverPacket = {0};
			serverPacket.packetType = PACKET_SEND_MESSAGE;
			serverPacket.messageFromID = packet.clientID;
			snprintf(serverPacket.message, sizeof serverPacket.message, "%.*s",
				(int)sizeof packet.message, packet.message);
			queueServerPacketAll(serverPacket, *gameState, stru);
		}


	}

	// update game state
	// send all players the current game state
	if (gameInfo.broadcastGameState) {
		printf("gameState has been updated, broadcasting...\n");
		queueGameState(*gameState, stru);
	}

	for(int y = 0; y < 8; ++y) {
		for(int x = 0; x < 8; ++x) {
			gameState->currentChessBoard.pieces[y][x].previousType = gameState->currentChessBoard.pieces[y][x].type;
			gameState->currentChessBoard.pieces[y][x].previousColor = gameState->currentChessBoard.pieces[y][x].color;
		}
	}

	return true;
}
