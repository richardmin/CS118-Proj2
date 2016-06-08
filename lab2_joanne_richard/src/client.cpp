#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <iostream>
#include <sstream>


#include "../utils/IPResolver.h"
#include "../utils/TCPManager.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../utils/TCPConstants.h"
#include <stdint.h>

// Tutorial regarding sending arbitrary packet frames. 
// http://www.microhowto.info/howto/send_a_udp_datagram_in_c.html
int main(int argc, char* argv[])
{
  srand(time(NULL));
   int portnum = -1;
   char* IP;
   //-------------- Parse Command Line Arguments ----------------//
   if(argc < 3)
   {
     std::cerr << "Usage: client [Server Host or IP] [Port-Number]" << std::endl;
     exit(1);
   }
	
   std::stringstream convert(argv[2]);
   if(!(convert >> portnum))
   {
     std::cerr << "[Port-Number] must be a valid integer" << std::endl;
     exit(1);
   }
  
  IP = (char*)malloc((strlen(argv[1]) + 1) * sizeof(char));
  if(IP == NULL)
  {
    std::cerr << "Out of memory" << std::endl;
    exit(1);
  }
  strcpy(IP, argv[1]);
  if(resolveIP_cstr(IP) == -1)
  {
    std::cerr << "IP Failed to resolve! IP passed in was" << argv[1] << std::endl;
    exit(1);
  }
  //-------------- File to save into -------------------//
  FILE* fp = fopen("received.data", "w"); //write-only: we're going to be writing to the file. Throw away what previously existed.
  if (fp == NULL) 
  {
    perror("open");
    std::cerr << "File unable to be opened." << std::endl;
    exit(2);
  }

  //-------------- Set up socket connection ------------//
  signal(SIGPIPE, SIG_IGN); //Ignore poorly terminated connections from terminating our server

  // create a socket with UDP
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd == -1)
  {
    perror("socket");
    exit(2);
  }

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(3);
  }

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(portnum); 
  addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  //bind the socket
  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    exit(4);
  }

  struct sockaddr_in remote_addr;
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_port = htons(portnum); 
  remote_addr.sin_addr.s_addr = inet_addr(IP); 
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  free(IP);

  // std::cout << "Portnum: " << htons(addr.sin_port) << std::endl;
  TCPManager t = TCPManager();
  t.custom_send(sockfd, fp, (struct sockaddr*) &remote_addr, (socklen_t) sizeof(remote_addr));

  close(sockfd);
}
