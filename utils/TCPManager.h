#ifndef TCPManager_h
#define TCPManager_h

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

class TCPManager
{
public:
	int custom_accept(int sockfd, struct sockaddr *addr, socklen_t* addrlen, int flags);
	int custom_connect(int sockfd, const struct sockaddr * addr, socklen_t addrlen);
	int custom_recv(int sockfd, void* buf, size_t len, int flags);
	int custom_send(int sockfd, void* buf, size_t len, int flags);
	TCPManager();
	~TCPManager();
private:
	int last_seq_num;
	int last_ack_num;
	struct timespec last_received_msg_time;

	bool connection_established;

	int next_seq_num(int datalen);
	int next_ack_num(int datalen);

	// int update_
}


#endif