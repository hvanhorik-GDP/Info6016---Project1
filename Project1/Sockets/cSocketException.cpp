#include "cSocketException.h"

#include <sstream>
#include <iostream>

cSocketException::cSocketException(int result)
	: std::exception()
	, m_result(result)
{
}

const char* cSocketException::what() const throw ()
{
	std::stringstream ss;
	ss << "Socket exception: Value = " << m_result;
	static std::string msg = ss.str();
	return msg.c_str();
}

void cSocketException::throwIfError(int result)
{
	if (result != 0) {
		std::cout << "Socket error: " << result << std::endl;
		throw cSocketException(result);
	}
}

void cSocketException::throwError(int result)
{
	std::cout << "Socket error: " << result << std::endl;
	throw cSocketException(result);
}

