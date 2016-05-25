#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <iostream>
#include <sstream>


#include "../utils/IPResolver.h"
#include "../utils/TCPManager.h"

// Tutorial regarding sending arbitrary packet frames. 
// http://www.microhowto.info/howto/send_a_udp_datagram_in_c.html
int main(int argc, char* argv[])
{
  if(argc < 3)
  {
    std::cerr << "Usage: client [Server Host or IP] [Port-Number]" << std::endl;
    exit(1);
  }
	int portnum = -1;
  char* IP = (char*)malloc((strlen(argv[1]) + 1) * sizeof(char));
  strcpy(IP, argv[1]);
  std::stringstream convert(argv[2]);
  if(!(convert >> portnum))
  {
    std::cerr << "[Port-Number] must be a valid integer" << std::endl;
    exit(1);
  }

  int r = resolveIP_cstr(IP);
  if(r == -1)
  {
    std::cerr << "IP Failed to resolve! IP passed in was argv[1]" << std::endl;
  }

  std::cerr << "web client is not implemented yet" << std::endl;
  // do your stuff here! or not if you don't want to.
  free(IP);
}
