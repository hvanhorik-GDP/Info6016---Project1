#include "cConnectSocket.h"
#include "cSocketException.h"

#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cassert>

using namespace std;

cConnectSocket::cConnectSocket(int backlog)
	: m_backlog(backlog)
{
}

cConnectSocket::~cConnectSocket()
{
	Close();
	Free();
}

void cConnectSocket::Free()		// NoThrow so safe on destruction
{
	if (m_addrInfo)
	{
		::freeaddrinfo(m_addrInfo);
		m_addrInfo = 0;
	}
}

void cConnectSocket::GetInfo(const std::string &ip, const std::string& port)
{
	struct addrinfo hints;

	// Define our connection address info 
	// Just default everything for this project
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	m_iResult = getaddrinfo(ip.c_str(), port.c_str(), &hints, &m_addrInfo);
	cSocketException::throwIfError(m_iResult);
	std::cout << *m_addrInfo;
}

cSocket* cConnectSocket::Connect()
{
	assert(m_addrInfo);
	// #2 connect
	struct addrinfo* ptr = NULL;
	// Attempt to connect to the server until a socket succeeds
	for (ptr = m_addrInfo; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to the server
		m_id = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (m_id == INVALID_SOCKET)
		{
			m_iResult = WSAGetLastError();
			cout << "socket() failed with error code: " << m_iResult << endl;
			cSocketException::throwError(m_iResult);
		}

		// Attempt to connect to the server
		m_iResult = connect(m_id, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (m_iResult == SOCKET_ERROR)
		{
			m_iResult = WSAGetLastError();
			cout << "connect() failed with error code: " << m_iResult << endl;
			closesocket(m_id);
			m_id = INVALID_SOCKET;
			continue;
		}
		break;
	}
	return new cSocket(m_id);
}
