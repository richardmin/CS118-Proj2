#ifndef TCPManager_h
#define TCPManager_h

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include "TCPConstants.h"

#define BILLION 1000000000L

class TCPManager
{
public:
	int custom_recv(int sockfd, FILE* fp);
	int custom_send(int sockfd, FILE* fp, const struct sockaddr* remote_addr, socklen_t remote_addrlen);

	TCPManager();
	~TCPManager();
private:
	uint16_t last_seq_num;
	uint16_t last_ack_num;

	uint16_t last_cumulative_seq_num;
	struct timespec last_received_msg_time;

	bool connection_established;

	uint16_t next_seq_num(int datalen);
	uint16_t next_ack_num(int datalen);

	int wait_for_packet();

	int timespec_subtract (struct timespec *result, struct timespec *y, struct timespec *x);
	void populateHeaders(void* buf, packet_headers &headers);

	bool compare_sockaddr(const struct sockaddr_in* sockaddr_1, const struct sockaddr_in* sockaddr_2);

	struct buffer_data data_buffer[MAX_SEQUENCE_NUMBER]; //array of all the possible windows, indexed by sequence number. 
	
	uint16_t ssthresh;
	uint16_t cwnd;
};


#endif