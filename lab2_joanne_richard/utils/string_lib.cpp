#include "string_lib.h"

#include <string>
#include <string.h>
#include <stdio.h>
#include <iostream>


/**
 * Function: char* stringToCString(std::string s)
 * Usage: stringToCString(std::string s)
 * ------------------------
 * Post-Conditions: returns a mutable c string version of the std::string.
 */
char* stringToCString(const std::string s)
{
	//convert the string to a const c string
	const char* s_cstr = s.data(); 

	//Allocate memory for the c string.
	char* s_cpy = (char *)malloc(sizeof(char) * (strlen(s_cstr)+1));
	
	//Allocate Failure.
	if(s_cpy == NULL)
	{
		std::cerr << "Malloc Failed" << std::endl;
		exit(1);
	}

	//Copy all the values from the constant string to the c string.
	for(uint i = 0; i <= strlen(s_cstr); i++)
		s_cpy[i] = s_cstr[i];
	
	return s_cpy;
}