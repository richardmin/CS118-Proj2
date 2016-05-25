#include "TCPManager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int custom_listen(int sockfd, int backlog)
{
	return -1;
}

int custom_accept(int sockfd, struct sockaddr *addr, socklen_t* addrlen, int flags)
{
	return -1;
}

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
