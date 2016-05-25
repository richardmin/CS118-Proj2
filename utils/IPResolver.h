#ifndef IPResolver_h
#define IPResolver_h

#include <string>
#include <stdint.h>

int resolveIP(std::string& hostname);
int resolveIP_cstr(char* hostname);


struct packet_headers {
	short h_seq;
	short h_ack;
	short h_window;
	short flags;
}

#define ACK_FLAG 0x100
#define SYN_FLAG 0x10
#define FIN_FLAG 0x1

#endif