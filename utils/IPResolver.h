#ifndef IPResolver_h
#define IPResolver_h

#include <string>

int resolveIP(std::string& hostname);
int resolveIP_cstr(char* hostname);

#endif