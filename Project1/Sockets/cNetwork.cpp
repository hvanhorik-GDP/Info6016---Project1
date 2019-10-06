#include "cNetwork.h"
#include "cSocketException.h"
#include "cListenSocket.h"

#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <cassert>

cNetwork::cNetwork() 
{
}

cNetwork::~cNetwork() 
{
	Cleanup();
}
void cNetwork::Initialize()
{
	WSADATA wsaData;

	// Initialize Winsock
	m_iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	cSocketException::throwIfError(m_iResult);
	std::cout << "WSAStartup() was successful!" << std::endl;
}

void cNetwork::Cleanup()
{
	if (m_listenSocket)
		delete m_listenSocket;
	m_listenSocket = NULL;
	if (m_connectsocket)
		delete m_connectsocket;
	m_connectsocket = NULL;
	for (mapSocket::iterator it = m_mapSockets.begin(); it != m_mapSockets.end(); ++it)
	{
		cSocket* sock = it->second;
		m_mapSockets.erase(it);
		delete sock;
	}
	if (m_isInitialized)
		::WSACleanup();
}

cListenSocket& cNetwork::CreateListenSocket(int backlog)
{
	assert(m_listenSocket == 0);
	m_listenSocket = new cListenSocket(backlog);
	return *m_listenSocket;
}

cConnectSocket& cNetwork::CreateConnectSocket()
{
	assert(m_connectsocket == 0);
	m_connectsocket = new cConnectSocket();
	return *m_connectsocket;	
}

void cNetwork::AddSocket(cSocket* in)
{
	assert(in);
	m_mapSockets[in->GetID()] = in;
}

void cNetwork::RemoveSocket(cSocket* in)
{
	mapSocket::iterator it = m_mapSockets.find(in->GetID());
	if (it != m_mapSockets.end())
	{
		cSocket* sock = it->second;
		sock->Close();
		m_mapSockets.erase(it);
		delete sock;
	}
}
