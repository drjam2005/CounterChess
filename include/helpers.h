#pragma once
#ifndef _HELPERS_H
#define _HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>

// Returns expectedSize on success, 0 on clean EOF before any bytes, or -1 on error.
// EOF after a partial read is reported as -1 with errno set to ECONNRESET.
ssize_t recvAll(int socketfd, void* buffer, size_t expectedSize, int flags);

// Returns expectedSize on success or -1 on error. Uses MSG_NOSIGNAL where available.
ssize_t sendAll(int socketfd, const void* buffer, size_t expectedSize, int flags);

#ifdef __cplusplus
}
#endif
#endif
