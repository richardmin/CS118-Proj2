#include "TCPManager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <iostream>

#include <stdlib.h>
#include <time.h>

TCPManager::TCPManager()
{
	srand(time(NULL)); //note that we must do this for our TCP ack/sequence numbers to be random
	last_syn_num = -1;
	last_ack_num = -1;
	connected_established = false;
}

~TCPManager::TCPManager()
{
}

/**
 * Function: custom_listen(int sockfd, int backlog)
 * Usage: custom_listen(sockfd, 1);
 * ------------------------------
 * This function does not block, listening for incoming SYNs on this socket?
 * This function blocks, listening for incoming ACKs and responds with a SYN-ACK. 
 * Backlog is ignored in our implementation ()
 */
int TCPManager::custom_listen(int sockfd, int backlog)
{
	
	return -1;
}

/* This function blocks, listening for incoming SYNs and responds with a SYN-ACK.*/
int TCPManager::custom_accept(int sockfd, struct sockaddr *addr, socklen_t* addrlen, int flags)
{
	char buffer[MAX_PACKET_LENGTH+1];
	ssize_t count = recvfrom(sockfd, buffer, sizeof(buffer), 0, addr, addrlen);
	if (count == -1) 
		return -1;
	else if (count == sizeof(buffer)) 
		std::cerr << "Datagram too large for buffer" << std::endl;
	else {
		//Check for SYN flag
		
	}
	
	return -1;
}

/**
 * Function: custom_connect(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
 * Usage: custom_connect(sockfd, &addr, sizeof(addr))
 * -----------------------------
 * This function sends a UDP SYN message, and waits for the SYN-ACK to be received. 
 */
int TCPManager::custom_connect(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
{
	/*
	packet_headers syn_packet = {rand(), 0, INIT_RECV_WINDOW, SYN_FLAG};
	return sendto(sockfd, syn_packet, PACKET_HEADER_LENGTH, , addr, addrlen) )
	*/
	return -1;
}

int TCPManager::custom_recv(int sockfd, void* buf, size_t len, int flags)
{
	return -1;
}

int TCPManager::custom_send(int sockfd, void* buf, size_t len, int flags)
{
	return -1;
}

