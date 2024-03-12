// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef TO_STRING
#define TO_STRING

#include <string>

inline void XorSwap( char* x, char* y ) {
  if (x != y) {
    *x ^= *y;
    *y ^= *x;
    *x ^= *y;
  }
}

inline char* ReverseString( char* string, unsigned int size ) {
    char element0;
	char element1;

	for ( unsigned int i = 0; i < size / 2; ++i) {
		element0 = string[i];
		element1 = string[size - i - 1];

		string[size - i - 1] = element0;
		string[i] = element1;
	}
	
	return string;
}

inline std::string ToString( int value ) {
	unsigned int stringIndex = 0;
	int var = 0;
	char buffer[12];

	if (value == 0) {
		buffer[stringIndex] = 48;
		++stringIndex;
	}
	
	while( value > 0 ) {
		var = value % 10;
		value = value / 10;
		var += 48;

		buffer[stringIndex] = var;
		++stringIndex;
	}
	
	ReverseString( buffer, stringIndex );
	buffer[stringIndex] = '\0';
	return std::string (buffer);
}

inline std::string ConcatIntBetweenTwoStrings(	std::string leftString, unsigned int value, std::string rightString ) {
	std::string resultString = leftString + ToString(value) + rightString;
	
	return resultString;
}

inline std::string ConcatIntBetweenTwoStrings(	std::string leftString, std::string rightString, unsigned int value  ) {
	std::string resultString = leftString + rightString + ToString(value);
	
	return resultString;
}

#endif
