#pragma once
#include <exception>

class cSocketException :
	public std::exception
{
public:
	cSocketException(int result);

	virtual const char* what() const throw ();

	static void throwIfError(int result);
	static void throwError(int result);

private:
	cSocketException();
	int m_result;
};

