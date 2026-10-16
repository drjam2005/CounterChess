#pragma once
#ifndef _HELPERS_H
#define _HELPERS_H

#include <sys/socket.h>

ssize_t recvAll(int socketfd, void* buffer, size_t expectedSize, int flags);
ssize_t sendAll(int socketfd, const void* buffer, size_t expectedSize, int flags);

#endif
