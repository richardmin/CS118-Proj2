#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>
#include <thread>
#include <iostream>
#include <sstream>
#include <thread>

// Tutorial regarding sending arbitrary packet frames. 
// http://www.microhowto.info/howto/send_an_arbitrary_ethernet_frame_using_an_af_packet_socket_in_c.html

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
    struct stat stat;
    fstat(fd, &stat);
    if (!S_ISREG(stat.st_mode)) //Make sure we can deliver file contents (Regular File)
    {
      std::cerr << "File " << argv[2] << " not a regular file" << std::endl;
      exit(2);
    }
  }

  //-------------- Set up socket connection ------------//
  signal(SIGPIPE, SIG_IGN); //Ignore poorly terminated connections from terminating our server

  // create a socket with custom protocol and connectionless state
  // http://linux.die.net/man/7/packet
  int sockfd = socket(AF_PACKET, SOCK_DGRAM, 0);
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
  struct sockaddr_ll addr;
  addr.sll_family = AF_PACKET;
  addr.sll_port = htons(portnum); 
  addr.sll_addr.s_addr = inet_addr(hostname_cstr);
  memset(addr.sll_zero, '\0', sizeof(addr.sin_zero));

  //bind the socket
  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    exit(5);
  }


}
