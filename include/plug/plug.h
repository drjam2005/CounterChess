#pragma once
#ifndef PLUG_H
#define PLUG_H
#include "../game_state.h"
#include "../my_structs.h"
#include "../packet_handler.h"

typedef struct RenderInfo RenderInfo;
typedef struct RenderMessage RenderMessage;
typedef struct PlugClient {
	const char* libpath;
	void *handle;
	void (*renderBoard)(const struct GameState, struct RenderInfo);
	void (*loadPieceTextures)(void);
	void (*unloadPieceTextures)(void);
	void (*renderMessages)(RenderMessage* messages, int, int, int);
	struct PiecePosition (*toBoardPosition)(int, int, enum PIECE_COLOR);
} PlugClient;

typedef struct GameInfo GameInfo;
typedef struct PlugServer {
	const char* libpath;
	void *handle;
	bool (*handleGameState)(GameState*, fromClientPacketQueue*, struct GameInfo, queueServerStruct);
} PlugServer;

void reloadPlugClient(PlugClient* plug);
void reloadPlugServer(PlugServer* plug);

#endif
