#include "TCPManager.h"
#include "TCPConstants.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <iostream>

#include <stdlib.h>
#include <time.h>
#include <thread>

TCPManager::TCPManager()
{
	srand(time(NULL)); //note that we must do this for our TCP ack/sequence numbers to be random
	last_seq_num = -1;
	last_ack_num = -1;
	connected_established = false;
}

~TCPManager::TCPManager()
{
}

/**
 * Function: custom_accept(int sockfd, int backlog)
 * Usage: custom_accept(sockfd, 1);
 * ------------------------------
 * This function blocks, listening for incoming ACKs and responds with a SYN-ACK. 
 * Backlog is ignored in our implementation (typically would be used for waiting for more connections on queue)
 */
int TCPManager::custom_accept(int sockfd, struct sockaddr *addr, socklen_t* addrlen, int flags)
{
	return -1;
}

/**
 * Function: custom_connect(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
 * Usage: custom_connect(sockfd, &addr, sizeof(addr))
 * -----------------------------
 * This function sends a UDP SYN message, and waits for the SYN-ACK to be received. 
 * It then sends a ACK. 
 */
int TCPManager::custom_connect(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
{

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


/**
 * Function: next_seq_num()
 * Usage: next_seq_num(datalen)
 * ------------------------
 * This function generates the next syn number: If it's uninitialized, it initializes it. 
 * Otherwise, it just returns what's stored. Upon receiving a packet one shoudl update last_syn_num
 * to the appropriate offset (based on byte length).
 * You pass in how big the packet you are sending is (datawise).
 * Note that syn/acks are one byte each.
 */

int TCPManager::next_seq_num(int datalen)
{
	//generate the first seq number
	if(last_seq_num == -1)
		last_seq_num = rand() % MAX_SEQUENCE_NUMBER;
	
	int cached_seq_num = last_seq_num;
	last_seq_num += datalen;
	if(last_seq_num >= MAX_SEQUENCE_NUMBER) //this might overflow if you pass in too large a number for datalen
		last_seq_num -= MAX_SEQUENCE_NUMBER;
	return cached_seq_num;
}

/**
 * Function: next_ack_num()
 * Usage: next_ack_num(len)
 * -------------------------
 * This function generates the next ack number. last_ack_num should be set by the connection.
 * 
 */
int TCPManager::next_ack_num(int datalen)
{
	if(!connected_established)
		return -1;
	int cached_ack_num = last_ack_num
	last_ack_num += datalen;
	if(last_seq_num >= MAX_SEQUENCE_NUMBER) //this might overflow if you pass in too large a number for datalen
		last_seq_num -= MAX_SEQUENCE_NUMBER;
	return last_ack_num;
}
