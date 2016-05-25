#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <thread>
#include <iostream>
#include <sstream>

#include "../utils/TCPManager.h"

// Tutorial regarding sending arbitrary packet frames. 
// http://www.microhowto.info/howto/listen_for_and_receive_udp_datagrams_in_c.html
// http://beej.us/net2/html/syscalls.html
int main(int argc, char* argv[])
{
  int portnum = -1;
  std::string filename;

  //-------------- Parse Command Line Arguments ----------------//
  if(argc < 3)
  {
    std::cerr << "Usage: server [Port-Number] [File-Name]" << std::endl;
    exit(1);
  }

  std::stringstream convert(argv[1]);
  if(!(convert >> portnum))
  {
    std::cerr << "[Port-Number] must be a valid integer" << std::endl;
    exit(1);
  }

  //------------- Open the file ----------------//
  FILE* fp = fopen(argv[2], "r"); //read-only: we're not going to be modifying the file at all.
  if (fp == NULL) //file not found 
  {
    perror("open");
    std::cerr << "File " << argv[2] << " not found" << std::endl;
    exit(2);
  }
  else //valid file
  {
    int fd = fileno(fp);
    struct stat stats;
    fstat(fd, &stats);
    if (!S_ISREG(stats.st_mode)) //Make sure we can deliver file contents (Regular File)
    {
      std::cerr << "File " << argv[2] << " not a regular file" << std::endl;
      exit(2);
    }
  }

  //-------------- Set up socket connection ------------//
  signal(SIGPIPE, SIG_IGN); //Ignore poorly terminated connections from terminating our server

  // create a socket with UDP
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd == -1)
  {
    perror("socket");
    exit(3);
  }

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(4);
  }

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(portnum); 
  addr.sin_addr.s_addr = inet_addr("10.0.0.1"); //use your own IP address. 
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  //bind the socket
  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    exit(5);
  }


  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(addr.sin_family, &addr.sin_addr, ipstr, sizeof(ipstr));
  
  //Output the automatically binded IP address.
  std::cerr << ipstr << std::endl;
  std::cerr << ntohs(addr.sin_port) << std::endl;


  close(sockfd);

}
