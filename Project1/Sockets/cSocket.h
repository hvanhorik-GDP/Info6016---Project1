#pragma once


#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#include <iostream>

#include <ostream>

struct addrinfo;
class cSocket
{
public:
	cSocket();
	cSocket(SOCKET id);
	virtual ~cSocket();
	void Close();		// NoThrow so safe on destruction
	void SetNonBlocking();
	std::size_t Recieve(char* buffer, std::size_t length);
	void Send(const char* buffer, std::size_t length);
	SOCKET GetID(); 

protected:
	SOCKET m_id = INVALID_SOCKET;
	bool m_isOpen;
	int m_isConnected;
	int m_iResult;
};

std::ostream& operator<<(std::ostream& stream, const addrinfo& info);
