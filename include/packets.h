#pragma once
#ifndef _PACKET_H
#define _PACKET_H

#include <string>

struct ClientMessage {
	int clientFD;
	int clientID;
	char message[256];
};

#endif // _PACKET_H
