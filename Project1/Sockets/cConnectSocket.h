#pragma once
#include "cSocket.h"
#include <string>

struct addrinfo;

class cConnectSocket :
	public cSocket
{
public:
	cConnectSocket(int backlog = 10);
	virtual ~cConnectSocket();
	void Free();
	void GetInfo(const std::string& ip, const std::string& port);
	cSocket* Connect();
private:
	int m_backlog;
	struct addrinfo* m_addrInfo = 0;
};

