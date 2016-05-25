#include "TCPManager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <iostream>

/**
 * Function: custom_listen(int sockfd, int backlog)
 * Usage: custom_listen(sockfd, 1);
 * ------------------------------
 * This function blocks, listening for incoming ACKs and responds with a SYN-ACK. 
 * Backlog is ignored in our implementation ()
 */
int custom_listen(int sockfd, int backlog)
{

	return -1;
}

int custom_accept(int sockfd, struct sockaddr *addr, socklen_t* addrlen, int flags)
{
	return -1;
}

/**
 * Function: custom_connect(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
 * Usage: custom_connect(sockfd, &addr, sizeof(addr))
 * -----------------------------
 * This function sends a UDP SYN message, and waits for the SYN-ACK to be received. 
 */
int custom_connect(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
{
	return -1;
}

int custom_recv(int sockfd, void* buf, size_t len, int flags)
{
	return -1;
}

int custom_send(int sockfd, void* buf, size_t len, int flags)
{
	return -1;
}

