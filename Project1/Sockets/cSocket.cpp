#include "cSocket.h"
#include "cSocketException.h"
#include <iostream>
#include <ostream>

#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#include <iostream>

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
	if (m_id != INVALID_SOCKET)
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

	stream
		<< "getaddrinfo() is good!" << std::endl
		<< "    flags: " << info.ai_flags << std::endl
		<< "    ai_family: " << info.ai_family << std::endl
		<< "    ai_socktype: " << info.ai_socktype << std::endl
		<< "    ai_protocol: " << info.ai_protocol << std::endl;

	return stream;
}

std::size_t cSocket::Recieve(char* buffer, std::size_t length)
{
	cout << "cSocket - " << "Waiting to receive data from the client..." << endl;
	m_iResult = recv(m_id, buffer, (int)length, 0);
	if (m_iResult > 0)
	{
		// We have received data successfully!
		// iResult is the number of bytes received
		cout << "cSocket - " << "Bytes received:" << m_iResult << endl;
	}
	else if (m_iResult < 0)
	{
		m_iResult = WSAGetLastError();
		cout << "cSocket - " << "Recieve failed with error: " << m_iResult << endl;
		cSocketException::throwError(m_iResult);
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
		cout << "cSocket - " << "Bytes sent: " << m_iResult << endl;
	}
}


SOCKET cSocket::GetID()
{
	return m_id;
}
