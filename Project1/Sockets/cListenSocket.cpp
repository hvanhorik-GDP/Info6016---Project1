#include "cListenSocket.h"
#include "cSocketException.h"

#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cassert>

cListenSocket::cListenSocket(int backlog)
	: m_backlog(backlog)
{
}

cListenSocket::~cListenSocket()
{
	Close();
	Free();
}

void cListenSocket::Free()		// NoThrow so safe on destruction
{
	if (m_addrInfo)
	{
		::freeaddrinfo(m_addrInfo);
		m_addrInfo = 0;
	}
}
void cListenSocket::GetInfo(const std::string& port)
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
	m_iResult = getaddrinfo(NULL, port.c_str(), &hints, &m_addrInfo);
	cSocketException::throwIfError(m_iResult);
	std::cout << *m_addrInfo;
}

void cListenSocket::Create()
{
	assert(m_addrInfo);
	// Create a SOCKET for connecting to the server
	m_id = socket(
		m_addrInfo->ai_family,
		m_addrInfo->ai_socktype,
		m_addrInfo->ai_protocol
	);

	if (m_id == INVALID_SOCKET)
	{
		m_iResult = WSAGetLastError();
		cSocketException::throwError(m_iResult);
	}
	std::cout << "socket() is created!" << std::endl;
}

//void cListenSocket::SetNonBlocking()
//{
//	u_long iMode = 1;
//
//	m_iResult = ioctlsocket(m_id, FIONBIO, &iMode);
//	if (m_iResult)
//	{
//		m_iResult = WSAGetLastError();
//		cSocketException::throwError(m_iResult);
//	}
////	rc = ioctl(listen_sd, FIONBIO, (char*)& on);
//
//}

void cListenSocket::Bind()
{
	// #2 Bind - Setup the TCP listening socket
	m_iResult = bind(
		m_id,
		m_addrInfo->ai_addr,
		(int)m_addrInfo->ai_addrlen
	);
	if (m_iResult == SOCKET_ERROR)
	{
		m_iResult = WSAGetLastError();
		Close();					// Close it up
		Free();
		cSocketException::throwError(m_iResult);
	}
	std::cout << "bind() is good!" << std::endl;
}

void cListenSocket::Listen()
{
	// #3 Listen
	m_iResult = listen(m_id, m_backlog);
	if (m_iResult == SOCKET_ERROR)
	{
		m_iResult = WSAGetLastError();
		Close();
		cSocketException::throwError(m_iResult);
	}
	std::cout << "listen() was successful!" << std::endl;
}
// #4 Accept		(Blocking call)

cSocket* cListenSocket::Accept()
{
//	std::cout << "Waiting for client to connect..." << std::endl;
	SOCKET acceptSocket = accept(m_id, NULL, NULL);
	if (acceptSocket == INVALID_SOCKET)
	{
		m_iResult = WSAGetLastError();
		if (m_iResult == WSAEWOULDBLOCK)
		{
			return NULL;
		}
		else
		{
			std::cout << "accept() failed with error: " << m_iResult;
			cSocketException::throwError(m_iResult);
		}
	}
	std::cout << "accept() is OK!" << std::endl;
	std::cout << "Accepted client on socket: " << acceptSocket << std::endl;
	return new cSocket(acceptSocket);
}