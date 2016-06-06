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

#include <string.h>	//memcpy()

#include <arpa/inet.h>

#include <map>
TCPManager::TCPManager()
{
	last_seq_num = NOT_IN_USE;
	last_ack_num = NOT_IN_USE;
    last_cumulative_seq_num = NOT_IN_USE;
	connection_established = false;
    cwnd = INIT_WINDOW_SIZE;
    ssthresh = INIT_SLOW_START_THRESH;

}

TCPManager::~TCPManager()
{
}

/* 
 * Function: custom_recv(int sockfd, FILE* fp)
 * Usage: custom_recv(sockfd,fp)
 * ------------------
 * sockfd is the socket to listen upon, fp is the FILE* that the server should be serve
 * This function blocks, listening for incoming SYNs and responds with a SYN-ACK.
 *
 * This function handles the entire server side of the code, taking a socket to listen upon and responding over the file pointer.
 * It listens for Syns, and replies with Syn-acks to the sender adddress, establishing a TCP connection, and upon the receiving of
 * and ack it starts to send the window data back with the congestion control targetted appropriately. 
 */
int TCPManager::custom_recv(int sockfd, FILE* fp)
{
    char buf[MAX_PACKET_LENGTH+1]; //will end up null terminated.

    //The client we're connecting to: we remember this for the rest of the connection status
    //If we receive packets from someone that isn't this address, we just drop them. 
    sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    sockaddr_in received_addr;
    socklen_t received_addrlen = sizeof(received_addr);
    uint16_t last_recv_window;
    bool ack_received = false, syn_received = false, file_complete = false, fin_ack_established = false;
    //Wait for someone to establish a connection through SYN. Ignore all other packets. 
    
    //wait for a packet
    //Note that the received data is HTONS already here, as blocking does so, and recvfrom is stupid
    while(!syn_received)
    {
        ssize_t count = recvfrom(sockfd, buf, MAX_PACKET_LENGTH, 0, (struct sockaddr *) &client_addr, &client_addrlen);

        if (count == -1) { 
            perror("recvfrom");
            std::cerr << "recvfrom() ran into error" << std::endl;
        }
        else if (count > MAX_PACKET_LENGTH)
            std::cerr << "Datagram too large for buffer" << std::endl;
        else {
            //Decompose the header data
            struct packet_headers received_packet_headers;
            populateHeaders(buf, received_packet_headers);
            
            last_seq_num = (received_packet_headers.h_seq);
            last_ack_num = (received_packet_headers.h_ack);
            
            if (!((received_packet_headers.flags) ^ SYN_FLAG)) //check that ONLY the syn flag is set.
            {
                std::cout << "Receiving SYN packet" << std::endl;
                syn_received = true;
            }
        }
    }


    //Send initial SYN-ACK, set timeout.
    packet_headers synack_packet = {next_seq_num(0), next_ack_num(1), INIT_RECV_WINDOW, SYN_FLAG | ACK_FLAG};
    
    struct timespec result;
    
    if (!sendto(sockfd, &synack_packet, PACKET_HEADER_LENGTH, 0, (struct sockaddr *) &client_addr, client_addrlen) ) {
        std::cerr << "Error: could not send synack_packet" << std::endl;
        return -1;
    }
    else
        std::cout << "Sending SYN-ACK " << synack_packet.h_ack << std::endl;

    //begin the timeout
    clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
    long bytes_in_transit = 0;
    // Wait for ACK, timeout to SYN-ACK
    while(!ack_received)
    {
        do
        {
            struct timespec tmp;
            clock_gettime(CLOCK_MONOTONIC, &tmp);
            //wait for a response quietly.
            client_addrlen = sizeof(client_addr);
            timespec_subtract(&result, &last_received_msg_time, &tmp);
            ssize_t count = recvfrom(sockfd, buf, MAX_PACKET_LENGTH, MSG_DONTWAIT, (struct sockaddr *) &received_addr, &received_addrlen); //non-blocking
            
            if(count == -1 && errno == EAGAIN)
            {
                continue;
            }
            else if (count == -1) { 
                std::cerr << "recvfrom() ran into error" << std::endl;
                continue;
            }
            else if(!compare_sockaddr(&client_addr, &received_addr))
            {
                //different source
                continue;
            }
            else if (count > MAX_PACKET_LENGTH) {
                std::cerr << "Datagram too large for buffer" << std::endl;
                continue;
            } 
            else
            {
                struct packet_headers received_packet_headers;
                populateHeaders(buf, received_packet_headers);

                last_seq_num = received_packet_headers.h_seq;
                last_ack_num = received_packet_headers.h_ack;
                last_recv_window = received_packet_headers.h_window;

                if (!(received_packet_headers.flags ^ (ACK_FLAG))) 
                {
                    ack_received = true;
                    std::cout << "Receiving ACK " << last_ack_num << std::endl;
                    break;
                }
                else if(!(received_packet_headers.flags ^ SYN_FLAG)) //SYN-ACK lost, and another SYN received. resend syn-ack.
                {
                    if ( !sendto(sockfd, &synack_packet, PACKET_HEADER_LENGTH, 0, (struct sockaddr *) &client_addr, client_addrlen) )  {
                        std::cerr << "Error: Could not send syn_packet" << std::endl;
                        return -1;
                    }
                    else
                    {
                        std::cout << "Sending SYN-ACK " << synack_packet.h_ack << " Retransmission" << std::endl;
                    }
                    clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
                }
            }

        } while(result.tv_nsec < 500000000); //500 milliseconds = 500000000 nanoseconds

        if(!ack_received)
        {
            if (! sendto(sockfd, &synack_packet, PACKET_HEADER_LENGTH, 0, (struct sockaddr *) &client_addr, client_addrlen) ) {
                std::cerr << "Error: could not send synack_packet" << std::endl;
                return -1;
            }
            else
            {
                std::cout << "Sending SYN-ACK " << synack_packet.h_ack <<  " Retransmission" << std::endl;
            }
            clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
        }

    }

    uint16_t seqnum = last_ack_num;
    uint16_t acknum = last_seq_num + 1;
    if(acknum > MAX_SEQUENCE_NUMBER)
        acknum -= MAX_SEQUENCE_NUMBER;
    uint16_t window_index = NOT_IN_USE;
    clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
    
    //Connection established, can begin sending data, according to window size.
    while( !data_packets.empty() || !file_complete) //three conditions to end server: client unexpected end, out of file data to send
    {
        struct timespec tmp;
        clock_gettime(CLOCK_MONOTONIC, &tmp);
        timespec_subtract(&result, &last_received_msg_time, &tmp);
        
        //check for timeout
        if(result.tv_nsec > 500000000)
        {
            //update window sizes
            if(in_slow_start())
            {
                ssthresh = cwnd/2;
                cwnd = MAX_PACKET_PAYLOAD_LENGTH;
            }
            else
            {
                cwnd = cwnd/2;
            }

            //resend the entire window
            for(auto it : data_packets) //automatically iterate over the map
            {
                //extract the packet headers.
                uint16_t sequence_num = htons(it.second.data[0] << 8 | it.second.data[1]);

                if ( !sendto(sockfd, &it.second.data, it.second.size + 8, 0, (struct sockaddr*) &client_addr, client_addrlen) )  {
                    std::cerr << "Error: Could not retrasmit data_packet" << std::endl;
                    return -1;
                }
                else
                {
                    std::cout << "Sending data packet " << sequence_num << " " << cwnd << " " << ssthresh << " Retransmission" << std::endl;
                }
            }

            //reset the timeout window
            clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
        }

        // if((bytes_in_transit + 1024) > cwnd)
        //     std::cout << "TOO MANY BYTES IN TRANSIT" << "bytes_in_transit (expected): " << bytes_in_transit + 1024 << " cwnd: " << cwnd << std::endl;
        // if(file_complete)
        //     std::cout << "FILE COMPLETE" << std::endl;
        // if((bytes_in_transit+1024) > last_recv_window)
        //     std::cout << "GREATER THAN RECEIVER WINDOW " << std::endl;
        while((bytes_in_transit + 1024) <= cwnd && !file_complete && (bytes_in_transit+1024) <= last_recv_window)
        {
            buffer_data b;
            //read the data from the disk
            int readnum = fread(b.data+8, sizeof(char), 1024,  fp);

            if(feof(fp))
                file_complete = true;
            if(readnum == 0)
                break;
            b.size = readnum + 8;

            //note that acknum never increases because we never get data from the client.
            packet_headers p = {seqnum, acknum, INIT_RECV_WINDOW, 0};
            copyHeaders(&p, &b.data);
            seqnum += readnum;
            if(seqnum > MAX_SEQUENCE_NUMBER)
                seqnum -= MAX_SEQUENCE_NUMBER;


            //send more packets!
            if ( !sendto(sockfd, b.data, readnum + 8, 0, (struct sockaddr*) &client_addr, client_addrlen) )  {
                std::cerr << "Error: Could not retrasmit data_packet" << std::endl;
                return -1;
            }
            else
            {
                std::cout << "Sending data packet " <<  p.h_seq << " " << cwnd << " " << ssthresh << std::endl;
            }

            std::cout << "saved to map at: " << p.h_seq + b.size - 8 << std::endl;
            data_packets.insert(std::pair<uint16_t, buffer_data>(p.h_seq, b)); //index by sequence number
            if(window_index == NOT_IN_USE)
            {
                window_index = p.h_seq;
            }

            bytes_in_transit += b.size - 8;
            
        }
        
        //check to see if we've received any ACKs
        ssize_t count;
        while((count = recvfrom(sockfd, buf, MAX_PACKET_LENGTH, MSG_DONTWAIT, (struct sockaddr *) &received_addr, &received_addrlen)) > 0) //non-blocking
        {
            if(!compare_sockaddr(&client_addr, &received_addr))
                continue;

            //received a packet, should process it.
            packet_headers received_packet_headers;                
            populateHeaders(buf, received_packet_headers);
            last_recv_window = received_packet_headers.h_window;
            last_seq_num = received_packet_headers.h_seq;
            last_ack_num = received_packet_headers.h_ack;

            // std::cout << "last_recv_window " << last_recv_window << std::endl;

            switch(received_packet_headers.flags)
            {
                case ACK_FLAG:  //ACKing a prior message
                    //remove that packet from the mapping
                    //note that we don't particularly care if the ack was received for an imaginary packet
                {
                    
                    // std::cout << "Window index: " << window_index << " ack number: " << received_packet_headers.h_ack << 
                    // " size: " <<  data_packets.find(received_packet_headers.h_ack)->second.size - 8 << std::endl;
                    // std::cout << "Receiving ACK Packet " << received_packet_headers.h_ack;
                    // // std::cout << "bytes_in_transit " << bytes_in_transit << std::endl;


                    //if the window is below the ack number, or the distance between the acks is more than the window size diff
                    //that means it's not a retransmission
                    std::map<uint16_t,buffer_data>::iterator itlow, itup, tmp;

                    if(window_index <= received_packet_headers.h_ack - count)
                    {
                        if(received_packet_headers.h_ack - count - window_index <= MAX_WINDOW_SIZE)
                        {
                            itlow = data_packets.lower_bound(window_index);
                            itup = data_packets.upper_bound(received_packet_headers.h_ack - count);
                            tmp = itlow;
                            long diff = 0;
                            do
                            {
                                diff += tmp->second.size - 8;
                                tmp++;
                            } while(tmp != itup);
                            std::cout << "diff " << diff << std::endl;
                            // std::cout << "itlow->first: "<<  itlow->first << "window_index: " << window_index<< std::endl;
                            //how much the window moved to the right
                            bytes_in_transit -= diff;
                            window_index += diff;

                            data_packets.erase(itlow, itup);
                            clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
                        }
                        else 
                        {
                            std::cout << " Retransmission";
                        }
                    }
                    else
                    {
                        if(window_index - received_packet_headers.h_ack - count>= MAX_WINDOW_SIZE)
                        {
                            itlow = data_packets.lower_bound(window_index);
                            itup = data_packets.upper_bound(received_packet_headers.h_ack - count);
        
        
                            int diff = MAX_SEQUENCE_NUMBER - (window_index - itup->first);
        
                            bytes_in_transit -= diff;
        
                            data_packets.erase(itlow, data_packets.end()); //delete to the end
                            itup = data_packets.upper_bound(received_packet_headers.h_ack - count);
                            data_packets.erase(data_packets.begin(), itup);
        
                            clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
                        }
                        else
                        {
                            std::cout << " Retransmission";
                        }
                        
                    }
                    std::cout << std::endl;

                    //ack successfully received
                    cwnd = in_slow_start() ? cwnd * 2 : cwnd + MAX_PACKET_PAYLOAD_LENGTH;

                    break;
                }
                default: //Unknown type of packet, just drop it. Client shouldn't be sending anything.
                    break;
            }   
        }

    }
    

    //TODO: fix these seq and ack numbers
    clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
    packet_headers fin_packet = {seqnum, acknum, INIT_RECV_WINDOW, FIN_FLAG};
    //send the inital FIN packet
    if ( !sendto(sockfd, &fin_packet, PACKET_HEADER_LENGTH, 0, (struct sockaddr *) &client_addr, client_addrlen) )  {
        std::cerr << "Error: Could not send fin_packet" << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Sending FIN " << seqnum << std::endl;
    }
    seqnum++;
    
    while (!fin_ack_established) {
		do
		{
            struct timespec tmp;
			clock_gettime(CLOCK_MONOTONIC, &tmp);
			//wait for a response quietly.
			timespec_subtract(&result, &last_received_msg_time, &tmp);
			ssize_t count = recvfrom(sockfd, buf, MAX_PACKET_LENGTH, MSG_DONTWAIT, (struct sockaddr *) &received_addr, &received_addrlen); //non-blocking
            if(count == -1 && errno == EAGAIN)
            {
                //no data received
                continue;
            }
			else if (count == -1) { 
				std::cerr << "recvfrom() ran into error" << std::endl;
				continue;
			}
            else if(!compare_sockaddr(&client_addr, &received_addr))
            {
                //different source
                continue;
            }
			else if (count > MAX_PACKET_LENGTH) {
				std::cerr << "Datagram too large for buffer" << std::endl;
				continue;
			} 
			else
			{

                struct packet_headers received_packet_headers;                
                populateHeaders(buf, received_packet_headers);

				if (!(received_packet_headers.flags ^ (FIN_FLAG | ACK_FLAG))) 
				{
					fin_ack_established = true;
                    std::cout << "Receiving FIN_ACK " << last_ack_num << std::endl;
					break;
				}
            }
		} while(result.tv_nsec < 500000000); //5 milliseconds = 50000000 nanoseconds
        
        if(!fin_ack_established)
        {
            //Resend the fin packet
            if ( !sendto(sockfd, &fin_packet, PACKET_HEADER_LENGTH, 0, (struct sockaddr *) &client_addr, client_addrlen) )  {
                std::cerr << "Error: Could not send fin_packet" << std::endl;
                return -1;
            }
            else
            {
                std::cout << "Sending FIN " <<  seqnum << " Retransmission" << std::endl;
            }

            clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
        }
        
    }
    

    
    packet_headers final_ack_packet = {seqnum, acknum, INIT_RECV_WINDOW, ACK_FLAG};
    //send the final ACK packet
    if ( !sendto(sockfd, &final_ack_packet, PACKET_HEADER_LENGTH, 0, (struct sockaddr *) &client_addr, client_addrlen) )  {
        std::cerr << "Error: Could not send final_ack_packet" << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Sending ACK" << std::endl;
        clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
    }

    do {
        struct timespec tmp;
        clock_gettime(CLOCK_MONOTONIC, &tmp);

		timespec_subtract(&result, &last_received_msg_time, &tmp);
		ssize_t count = recvfrom(sockfd, buf, MAX_PACKET_LENGTH, MSG_DONTWAIT, (struct sockaddr *) &received_addr, &received_addrlen); 
        if(count == -1 && errno == EAGAIN)
        {
            //no data received
            continue;
        }
        else if (count == -1) { 
            std::cerr << "recvfrom() ran into error" << std::endl;
            continue;
        }
        else if(!compare_sockaddr(&client_addr, &received_addr))
        {
            //different source
            continue;
        }
        else if (count > MAX_PACKET_LENGTH) {
            std::cerr << "Datagram too large for buffer" << std::endl;
            continue;
        } 
        else
        {

            struct packet_headers received_packet_headers;                
            populateHeaders(buf, received_packet_headers);

            if (!(received_packet_headers.flags ^ (FIN_FLAG | ACK_FLAG))) 
            {
                if ( !sendto(sockfd, &final_ack_packet, PACKET_HEADER_LENGTH, 0, (struct sockaddr *) &client_addr, client_addrlen) )  {
                    std::cerr << "Error: Could not send final_ack_packet" << std::endl;
                    return -1;
                }
                else
                {
                    std::cout << "Sending ACK" << final_ack_packet.h_ack << " Retransmission" << std::endl;
                    clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
                }
            }
        }

    } while (result.tv_nsec < 5000000000);
    return 0;
}

/**
 * Function: custom_send(int sockfd, FILE* fp, const struct *sockaddr *remote_addr, socklen_t remote_addrlen
 * Usage: custom_send(sockfd, fp, &remote_addr, remote_addrlen)
 * -----------------------------
 * sockfd is the socket to send the request out upon/receive the request on; fp is the file to save the results to, remote_addr 
 * is the remote address for the socket, remote_addrlen is the length of that sturct
 * 
 * This function manages the entire client side of the code: sending out a SYN, waiting for a SYN-ACK and replying with the ACK and
 * receiving data. Also handles the closing of the connection: it blocks. 
 */
int TCPManager::custom_send(int sockfd, FILE* fp, const struct sockaddr *remote_addr, socklen_t remote_addrlen)
{

	packet_headers syn_packet = {next_seq_num(0), (uint16_t)NOT_IN_USE, cwnd, SYN_FLAG};

	char buf[MAX_PACKET_LENGTH];

	sockaddr_in client_addr;
	socklen_t client_addrlen = sizeof(client_addr);

	struct timespec result;
	bool message_received = false;

    //send the inital syn packet
    if ( !sendto(sockfd, &syn_packet, PACKET_HEADER_LENGTH, 0, remote_addr, remote_addrlen) )  {
        std::cerr << "Error: Could not send syn_packet" << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Sending SYN" << std::endl;
    }

    clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);

	while(!message_received)
	{
		do
		{
			struct timespec tmp;
			clock_gettime(CLOCK_MONOTONIC, &tmp);
			//wait for a response quietly.
			timespec_subtract(&result, &last_received_msg_time, &tmp);
			ssize_t count = recvfrom(sockfd, buf, MAX_PACKET_LENGTH, MSG_DONTWAIT, (struct sockaddr *) &client_addr, &client_addrlen); //non-blocking
            if(count == -1 && errno == EAGAIN)
            {
                //no data received
                continue;
            }
			else if (count == -1) { 
				std::cerr << "recvfrom() ran into error" << std::endl;
				continue;
			}
            else if(!compare_sockaddr(&client_addr, (sockaddr_in *)remote_addr))
            {
                //different source
                continue;
            }
			else if (count > MAX_PACKET_LENGTH) {
				std::cerr << "Datagram too large for buffer" << std::endl;
				continue;
			} 
			else
			{

                struct packet_headers received_packet_headers;                
                populateHeaders(buf, received_packet_headers);

                last_seq_num = received_packet_headers.h_seq;
                last_ack_num = received_packet_headers.h_ack;

				if (!(received_packet_headers.flags ^ (ACK_FLAG | SYN_FLAG))) 
				{
					message_received = true;
                    std::cout << "Receiving SYN-ACK " << last_ack_num << std::endl;
					break;
				}
			// std::cerr << result.tv_nsec << std::endl;
            }
		} while(result.tv_nsec < 500000000); //5 milliseconds = 50000000 nanoseconds

        if(!message_received)
        {
            //send the inital syn packet
            if ( !sendto(sockfd, &syn_packet, PACKET_HEADER_LENGTH, 0, remote_addr, remote_addrlen) )  {
                std::cerr << "Error: Could not send syn_packet" << std::endl;
                return -1;
            }
            else
            {
                std::cout << "Sending SYN Retransmission" << std::endl;
            }

            clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
        }
	}

	//Send ACK-Packet
	packet_headers ack_packet = {last_ack_num, (uint16_t)(last_seq_num + 1), INIT_RECV_WINDOW, ACK_FLAG};
	if ( !sendto(sockfd, &ack_packet, PACKET_HEADER_LENGTH, 0, remote_addr, remote_addrlen) ) {
		std::cerr << "Error: Could not send ack_packet" << std::endl;
		return -1;
	}
    else
    {
        std::cout << "Sending ACK " << ack_packet.h_ack << std::endl;
    }

    //Now we set up the connection data transfer, and wait for a fin
    //note that last_received_msg_time is being repurposed to the oldest packet's time.
    
    bool fin_established = false;
    uint16_t seqnum = last_ack_num; //immutable
    uint16_t acknum = last_seq_num + 1; //this is also the expected next sequence number
    packet_headers finack_packet;
    uint16_t window_index = acknum + 1024;
    uint16_t cached_ack = NOT_IN_USE;
    while(!fin_established)
    {
        //note that we have no timeout window; the server handles timeouts for us.
        ssize_t count = recvfrom(sockfd, buf, MAX_PACKET_LENGTH, MSG_DONTWAIT, (struct sockaddr *) &client_addr, &client_addrlen); //non-blocking

        if(count == -1 && errno == EAGAIN)
        {
            //no data received
            continue;
        }
        else if (count == -1) { 
            std::cerr << "recvfrom() ran into error" << std::endl;
            continue;
        }
        else if (count > MAX_PACKET_LENGTH) {
            std::cerr << "Datagram too large for buffer" << std::endl;
            continue;
        } 
        else if(!compare_sockaddr(&client_addr, (sockaddr_in *) remote_addr))
        {
            //different source, ignore
            continue;
        }   

        //received a packet, should process it.
        struct packet_headers received_packet_headers;                
        populateHeaders(buf, received_packet_headers);


        switch(received_packet_headers.flags)
        {
            case ACK_FLAG | SYN_FLAG: //ACK_SYN, resubmit ACK. We don't actually track in our window, so we inflate slightly slower!
            {
                if ( !sendto(sockfd, &ack_packet, PACKET_HEADER_LENGTH, 0, remote_addr, remote_addrlen) ) {
                    std::cerr << "Error: Could not send ack_packet" << std::endl;
                    return -1;
                }
                else
                {
                    std::cout << "Sending ACK " << ack_packet.h_ack << " Retransmission" << std::endl;
                }
                break;
            }
            case FIN_FLAG:  //FIN flag. Send back a FIN_ACK.
            {
                finack_packet = {seqnum, (uint16_t)(received_packet_headers.h_seq + 1), INIT_RECV_WINDOW, FIN_FLAG | ACK_FLAG}; //hacky
                if ( !sendto(sockfd, &finack_packet, PACKET_HEADER_LENGTH, 0, remote_addr, remote_addrlen) ) {
                    std::cerr << "Error: Could not send finack_packet" << std::endl;
                    return -1;
                }
                else
                {
                    std::cout << "Sending FIN_ACK " << finack_packet.h_ack << std::endl;
                }
                fin_established = true;
                clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
                break;
            }
            default: 
            {
                //Should be a data packet, we don't care about an ACK or to our data messages
                //This is because the client would time out and send the appropriate data back. 
                //remove that packet from the mapping of our window
                //note that we don't particularly care if the ack was received for an imaginary packet
                std::cout << "Receiving data packet " << received_packet_headers.h_seq;
                //calculate the ack to send, etc.

                uint16_t packet_ack = (received_packet_headers.h_seq + count - 8);
                if(window_index == packet_ack) //expected packet, in order. Write to disk.
                {
                    window_index += count - 8;
                    if(window_index > MAX_SEQUENCE_NUMBER)
                        window_index -= MAX_SEQUENCE_NUMBER;
                    fwrite(buf+8, sizeof(char), count - 8, fp); //write the received data to stream.

                    auto search = data_packets.find(window_index);
                    while(search != data_packets.end()) //write all the appropriate bits
                    {
                        fwrite(search->second.data + 8, sizeof(char), search->second.size, fp); //write the cached data to stream.
                        window_index += search->second.size;
                        if(window_index > MAX_SEQUENCE_NUMBER)
                            window_index -= MAX_SEQUENCE_NUMBER;
                        data_packets.erase(search);
                        search = data_packets.find(window_index);
                    }
                    acknum = window_index;
                }
                else if(window_index < packet_ack) 
                {
                    if(received_packet_headers.h_ack - window_index <= MAX_WINDOW_SIZE)//data we received is ahead of our window
                    {
                        //add the data into the map, with their ack numbers
                        buffer_data b;
                        b.size = count - 8;
                        memcpy(b.data, buf, count);

                        data_packets.insert(std::pair<uint16_t, buffer_data>(packet_ack, b)); //index by ack number
                    }
                    else 
                    {
                        std::cout << " Retransmission";
                    }
                }
                else
                {
                    if(window_index - received_packet_headers.h_ack >= MAX_WINDOW_SIZE) //data we received is ahead of our window
                    {
                        buffer_data b;
                        b.size = count - 8;
                        memcpy(b.data, buf, count);

                        data_packets.insert(std::pair<uint16_t, buffer_data>(packet_ack, b)); //index by ack number
                    }
                    else
                    {
                        std::cout << " Retransmission";
                    }
                    
                }
                std::cout << std::endl;
                std::cout << "window_index: " << window_index << " packet_ack: " << packet_ack << std::endl;


                packet_headers packet = {seqnum, packet_ack, INIT_RECV_WINDOW, ACK_FLAG}; //hacky
                if ( !sendto(sockfd, &packet, PACKET_HEADER_LENGTH, 0, remote_addr, remote_addrlen) ) {
                    std::cerr << "Error: Could not send ack_packet" << std::endl;
                    return -1;
                }
                else
                {
                    std::cout << "Sending ACK " << packet.h_ack;
                    if(cached_ack == packet.h_ack)
                        std::cout << " Retransmission";
                    std::cout << std::endl;
                    cached_ack = packet.h_ack;
                }
                //Send out ack for the packet received
                break;
            }
        }

    }

    //wait for a possible FIN retransmit
    do {
        struct timespec tmp;
        clock_gettime(CLOCK_MONOTONIC, &tmp);

        timespec_subtract(&result, &last_received_msg_time, &tmp);
        ssize_t count = recvfrom(sockfd, buf, MAX_PACKET_LENGTH, MSG_DONTWAIT, (struct sockaddr *) &client_addr, &client_addrlen); 
        if(count == -1 && errno == EAGAIN)
        {
            //no data received
            continue;
        }
        else if (count == -1) { 
            std::cerr << "recvfrom() ran into error" << std::endl;
            continue;
        }
        else if(!compare_sockaddr(&client_addr, (sockaddr_in *)remote_addr))
        {
            //different source
            continue;
        }
        else if (count > MAX_PACKET_LENGTH) {
            std::cerr << "Datagram too large for buffer" << std::endl;
            continue;
        } 
        else
        {

            struct packet_headers received_packet_headers;                
            populateHeaders(buf, received_packet_headers);

            if (!(received_packet_headers.flags ^ (FIN_FLAG))) 
            {
                if ( !sendto(sockfd, &finack_packet, PACKET_HEADER_LENGTH, 0, (struct sockaddr *) &client_addr, client_addrlen) )  {
                    std::cerr << "Error: Could not send final_ack_packet" << std::endl;
                    return -1;
                }
                else
                {
                    std::cout << "Sending FIN_ACK " << finack_packet.h_ack << " Retransmission" << std::endl;
                    clock_gettime(CLOCK_MONOTONIC, &last_received_msg_time);
                }
            }
        }

    } while (result.tv_nsec < 5000000000);
    return 0;

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

uint16_t TCPManager::next_seq_num(int datalen)
{
	//generate the first seq number, when no ack has yet been received. 
	if(last_ack_num == NOT_IN_USE)
    {
        last_cumulative_seq_num = rand() % MAX_SEQUENCE_NUMBER;
		return last_cumulative_seq_num;
    }

    //sequence numbers are cumulative: once you've sent the data the sequence number will go up
	uint16_t cache_seq_num = last_ack_num;
	cache_seq_num += datalen;
	if (cache_seq_num >= MAX_SEQUENCE_NUMBER)
			cache_seq_num -= MAX_SEQUENCE_NUMBER;
    if(last_cumulative_seq_num >= cache_seq_num)
        cache_seq_num += last_cumulative_seq_num; //acks are accumulative, so we should store the amount of data. 
    last_cumulative_seq_num = cache_seq_num;
	return last_cumulative_seq_num;
}


/**
 * Function: next_ack_num()
 * Usage: next_ack_num(len)
 * -------------------------
 * This function generates the next ack number. last_seq_num should be set by the connection, otherwise it returns an error. 
 * 
 */
// Next ack num = last sequence number received + amount of data received
uint16_t TCPManager::next_ack_num(int datalen)
{
    if(last_seq_num == NOT_IN_USE)
        return -1; //error, this function should not yet be called.
                    //acks are only cumulative based on the data that has been received: that is, the data that has been returned in sequence.

	//Next ack number will be the recieved_seqNum + datalen
	uint16_t next_ack_num = last_seq_num + datalen;
	if (next_ack_num >= MAX_SEQUENCE_NUMBER)
		next_ack_num -= MAX_SEQUENCE_NUMBER;

	return next_ack_num;
}


/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */
   //Modified From http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
int TCPManager::timespec_subtract (struct timespec *result, struct timespec *y, struct timespec *x)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_nsec < y->tv_nsec) {
    int nsec = (y->tv_nsec - x->tv_nsec) / BILLION + 1;
    y->tv_nsec -= BILLION * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_nsec - y->tv_nsec > BILLION) {
    int nsec = (x->tv_nsec - y->tv_nsec) / BILLION;
    y->tv_nsec += BILLION * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_nsec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_nsec = x->tv_nsec - y->tv_nsec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}


/* 
 * Function: populateHeaders(void* buf, packet_headers &headers)
 * Usage: populateHeaders(buf, headers)
 * --------------------------------
 * This function receives a buffer (that should be acquired from recvfrom, with minimum 8 bytes), taking the first 8 bytes 
 * and casting them to the struct. This will parse the headers and store them into the struct that's passed in.
 * 
 * WARNING: This does not safety checking: void*buf MUST be long enough as well and valid as well as headers must exist.
 */
void TCPManager::populateHeaders(void* buf, packet_headers &headers)
{
	char* buff = (char *) buf;
	headers.h_seq    = htons(buff[0] << 8 | buff[1]);
    headers.h_ack    = htons(buff[2] << 8 | buff[3]); 
    headers.h_window = htons(buff[4] << 8 | buff[5]);
    headers.flags    = htons(buff[6] << 8 | buff[7]); 
}

/*
 * Returns true if the passed in sockaddresses have the same port, address, and family.
 */
bool TCPManager::compare_sockaddr(const struct sockaddr_in* sockaddr_1, const struct sockaddr_in* sockaddr_2)
{
    return sockaddr_1->sin_port == sockaddr_2->sin_port  //same port number
            && sockaddr_1->sin_addr.s_addr == sockaddr_2->sin_addr.s_addr //same source address
            && sockaddr_1->sin_family == sockaddr_2->sin_family;
}

bool TCPManager::in_slow_start()
{
    return cwnd < ssthresh;
}

void TCPManager::copyHeaders(void* header, void* buffer)
{
    for(int i = 0; i < 8; i++)
    {
        ((char *)buffer)[i] = ((char *)header)[i];
    }
}

void TCPManager::copyData(void* header, void* buffer, int size)
{
    for(int i = 8; i < size+8; i++)
    {
        ((char *)buffer)[i] = ((char *)header)[i];
    }
}