#include "cSocket.h"
#include "cSocketException.h"
#include <iostream>
#include <ostream>

#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#include <iostream>

extern bool gDebug;					// Check for global debug

using namespace std;

cSocket::cSocket()
{
}

cSocket::cSocket(SOCKET id)
	: m_id(id)
{

}

cSocket::~cSocket()
{
	Close();
}

void cSocket::Close() 	// NoThrow so safe on destruction
{
	if (m_id != INVALID_SOCKET && m_isOpen)
	{
		int result = closesocket(m_id);
		// No exception on close
		if (result != 0)
		{
			std::cout << "closesocket() - failed with error: " << result << std::endl;
		}
	}
	m_id = INVALID_SOCKET;
}

std::ostream& operator<<(std::ostream& stream, const addrinfo& info)
{
	if (gDebug)
	{
		stream
			<< "getaddrinfo() is good!" << std::endl
			<< "    flags: " << info.ai_flags << std::endl
			<< "    ai_family: " << info.ai_family << std::endl
			<< "    ai_socktype: " << info.ai_socktype << std::endl
			<< "    ai_protocol: " << info.ai_protocol << std::endl;
	}
	return stream;
}

std::size_t cSocket::Recieve(char* buffer, std::size_t length)
{
//	cout << "cSocket - " << "Waiting to receive data from the client..." << endl;
	m_iResult = recv(m_id, buffer, (int)length, 0);
	if (m_iResult > 0)
	{
		// We have received data successfully!
		// iResult is the number of bytes received
		if (gDebug)
			cout << "cSocket - " << "Bytes received:" << m_iResult << endl;
	}
	else if (m_iResult < 0)
	{
		m_iResult = WSAGetLastError();
		if (m_iResult != WSAEWOULDBLOCK)
		{
			cout << "cSocket - " << "Recieve failed with error: " << m_iResult << endl;
			m_iResult = 0;				// Just fake no data
		}
		return m_iResult;
	}
	else // m_iResult == 0
	{
		cout << "cSocket - " << "Connection closing..." << endl;
	}
	return m_iResult;
}

void cSocket::Send(const char* buffer, std::size_t length)
{
	// Send data to the client
	m_iResult = send(m_id, buffer, (int)length, 0);
	if (m_iResult == SOCKET_ERROR)
	{
		m_iResult = WSAGetLastError();
		cout << "cSocket - " << "Send failed with error: " << m_iResult << endl;
	}
	else
	{
		if (gDebug)
			cout << "cSocket - " << "Bytes sent: " << m_iResult << endl;
	}
}

void cSocket::SetNonBlocking()
{
	//-------------------------
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the 
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled; 
	// If iMode != 0, non-blocking mode is enabled.
	u_long iMode = 1;

	m_iResult = ioctlsocket(m_id, FIONBIO, &iMode);
	if (m_iResult)
	{
		m_iResult = WSAGetLastError();
		cSocketException::throwError(m_iResult);
	}
}

SOCKET cSocket::GetID()
{
	return m_id;
}
