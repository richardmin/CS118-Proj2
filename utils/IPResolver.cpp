#include "string_lib.h"
#include "IPResolver.h"
#include <stdio.h>
#include <string>
#include <string.h> //memset
#include <cstdlib> //realloc
#include <iostream>

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include <arpa/inet.h>



/**
 * Function: resolveIP(std::string& hostname)
 * Usage: resolveIP(hostname)
 * --------------------------------
 * Post-conditions: hostname is a string that contains one of the IPs that the hostname passed in to the function resolves to.
 * 				         	Note that if the IP address is unresolvable the function exits horribly.
 * 
 * This function is a wrapper for the C String version of the function.
 */
void resolveIP(std::string& hostname)
{
  char* hostname_cstr = stringToCString(hostname);
  
  resolveIP_cstr(hostname_cstr);  
  hostname = std::string(hostname_cstr);
  
  free(hostname_cstr);
}

/**
 * Function: resolveIP(char* hostname)
 * Usage: resolveIP(hostname)
 * --------------------------------
 * Post-conditions: hostname is a c-string that contains one of the IPs that the hostname passed in to the function resolves to.
 * 					        Note that if the IP address is unresolvable the function exits horribly.
 * 
 * This function chooses the FIRST ip address that the hostname passed in resolves to. 
 */
void resolveIP_cstr(char* hostname)
{
  struct addrinfo hints;
  struct addrinfo* res;

  // prepare hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP

  // get address
  int status = 0;
  if ((status = getaddrinfo(hostname, "80", &hints, &res)) != 0) {
    std::cerr << "couldn't resolve IP address: " << gai_strerror(status) << std::endl;
    exit(1);
  }

    // convert address to IPv4 address
  struct sockaddr_in* ipv4 = (struct sockaddr_in*)res->ai_addr;

    // convert the IP to a string and print it:
  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(res->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));

  if(strlen(hostname) < strlen(ipstr)) //not enough space to hold ipstr
    if(realloc(hostname, (sizeof(char) + 1) * strlen(ipstr)) != 0){
      std::cerr << "Realloc Failed" << std::endl;
      exit(1);
    }
  
  strcpy(hostname, ipstr);

  freeaddrinfo(res); // free the linked list
}



