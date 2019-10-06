#pragma once

#include "cSocket.h"
#include "cListenSocket.h"
#include "cConnectSocket.h"
#include <map>

class cNetwork
{
public:
	typedef std::map<SOCKET, cSocket*> mapSocket;
	cNetwork();
	~cNetwork();

	void Initialize();
	void Cleanup();

	cListenSocket& CreateListenSocket(int backlog = 10);
	cConnectSocket& CreateConnectSocket();
	void AddSocket(cSocket* in);
	void RemoveSocket(cSocket* out);

private:
	bool m_isInitialized = false;
	int m_iResult;
	cListenSocket* m_listenSocket = NULL;
	cConnectSocket* m_connectsocket = NULL;
	mapSocket m_mapSockets;
};

