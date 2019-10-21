#include "cSocket.h"
#include "cSocketException.h"
#include "../Chat/cCommand.h"
#include <iostream>
#include <ostream>
#include <cassert>

#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#include <iostream>

extern bool gDebug;					// Check for global debug
extern bool gDebugSend;				// Checking for breaking up a message

using namespace std;

cSocket::cSocket()
{
	m_recieveBufferLength = cCommand::GetMaxRawLength() * 3;
	m_recieveBuffer = new char[m_recieveBufferLength];
	m_currentPosition = m_recieveBuffer;
}

cSocket::cSocket(SOCKET id)
	: m_id(id)
{
	m_recieveBufferLength = cCommand::GetMaxRawLength() * 3;
	m_recieveBuffer = new char[m_recieveBufferLength];
	m_currentPosition = m_recieveBuffer;

}

cSocket::~cSocket()
{
	Close();
	delete[] m_recieveBuffer;
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
	assert(m_recieveBuffer);
	std::size_t recievedSoFar = m_currentPosition - m_recieveBuffer;
	std::size_t maxLengthToReieve = m_recieveBufferLength - recievedSoFar;

	//	cout << "cSocket - " << "Waiting to receive data from the client..." << endl;
	m_iResult = recv(m_id, m_currentPosition, (int)maxLengthToReieve, 0);
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
		return m_iResult;
	}

	// We recieved something. Let's ensure that it is a full buffer
	m_currentPosition += m_iResult;
	std::size_t lengthSoFar = m_currentPosition - m_recieveBuffer;

	// Overflow
	assert(lengthSoFar <= m_recieveBufferLength);

	// Receive until we get a full header
	if (lengthSoFar < cCommand::GetHeaderLength())
	{
		cout << endl << "Recieved less than the full header: " << lengthSoFar << endl;
		m_iResult = WSAEWOULDBLOCK;
		return m_iResult;
	}

	// Check the buffer length for under/overflow
	std::size_t lengthToRecieve = cCommand::UnPackuLongBE(m_recieveBuffer);
	if (lengthToRecieve > cCommand::GetMaxRawLength())
	{
		// This will shut down the socket
		cout << endl << "Protocol error - Message too large: " << lengthToRecieve << endl;
		return -1;
	}

	// See if we have recieved everything they have said they will send
	// Note - if they lied about it the protocol will stall
	if (lengthSoFar < lengthToRecieve)
	{
		cout << endl << "Recieved less than the full buffer: " << m_iResult << endl;
		m_iResult = WSAEWOULDBLOCK;
		return m_iResult;
	}

	// Copy the data into the user buffer
	std::memcpy(buffer, m_recieveBuffer , lengthToRecieve);

	// If we have more data, it is part of the next buffer
	// More in buffer - Move it to the beginning  
	std::size_t bytesLeftOver = lengthToRecieve - lengthSoFar;
	if (bytesLeftOver)
	{
		cout << endl << "Recieved buffer overflow: " << bytesLeftOver << endl;
		std::memcpy(m_recieveBuffer, m_currentPosition, bytesLeftOver);
	}
	m_currentPosition = m_recieveBuffer;

	m_iResult = (int)lengthToRecieve;
	return m_iResult;
}

void cSocket::Send(const char* buffer, std::size_t length)
{
	if (gDebugSend)
	{
		// Testing for partial buffers
		m_iResult = send(m_id, buffer, 3, 0);
		Sleep(50);
		m_iResult = send(m_id, buffer + 3, 10, 0);
		Sleep(50);
		m_iResult = send(m_id, buffer + 13, (int)length - 13, 0);
		return;
	}
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
