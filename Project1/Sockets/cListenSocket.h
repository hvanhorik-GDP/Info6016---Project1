#pragma once
#include "cSocket.h"
#include <string>

struct addrinfo;

class cListenSocket :
	public cSocket
{
public:
	cListenSocket(int backlog = 10);
	virtual ~cListenSocket();
	void Free();
	void GetInfo(const std::string& port);
//	virtual void SetNonBlocking();
	void Create();
	void Bind();
	void Listen();
	cSocket* Accept();
private:
	int m_backlog;
	struct addrinfo* m_addrInfo = 0;
};

