#ifndef TCPConstants_h
#define TCPConstants_h

#include <stdint.h>
#include <time.h>

#define PACKET_HEADER_LENGTH 8
#define MAX_PACKET_PAYLOAD_LENGTH 1024 //Strangely, this is MSS
#define MAX_PACKET_LENGTH PACKET_HEADER_LENGTH + MAX_PACKET_PAYLOAD_LENGTH

#define MAX_SEQUENCE_NUMBER 30720
#define INIT_WINDOW_SIZE 1024
#define INIT_SLOW_START_THRESH 30720
#define RTT_VAL 500
#define INIT_RECV_WINDOW 30720
#define MAX_WINDOW_SIZE MAX_SEQUENCE_NUMBER/2

#define ACK_FLAG 0x4
#define SYN_FLAG 0x2
#define FIN_FLAG 0x1

#define NOT_IN_USE 65535 //this is UINT_MAX

struct packet_headers {
	uint16_t h_seq;
	uint16_t h_ack;
	uint16_t h_window;
	uint16_t flags;
};

struct buffer_data {
	int size;
	char data[1033];
};


#endif