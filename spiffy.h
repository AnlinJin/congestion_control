#ifndef _SPIFFY_H_
#define _SPIFFY_H_

#include <sys/types.h>
#include <sys/socket.h>
#include "debug.h"

struct spiffy_header_s {
	int ID;
	int lSrcAddr;
	int lDestAddr;
	short lSrcPort;
	short lDestPort;
};
typedef struct spiffy_header_s spiffy_header;

ssize_t spiffy_sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);

//This function has the same return value as the recvfrom function. It returns the number of bytes received
int spiffy_recvfrom (int socket, void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t *lengthptr);

//initialize the spiffy based on the SPIFFY_ROUTER environment variable. init glSrcAddr amd gsSrcPort variable
int spiffy_init (long lNodeID, const struct sockaddr *addr, socklen_t addrlen);

#endif /* _SPIFFY_H_ */
