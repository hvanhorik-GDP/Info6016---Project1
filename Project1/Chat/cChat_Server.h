#pragma once
#include "cChat.h"
#include "../Sockets/cNetwork.h"
#include "cCommand.h"
#include "../Sockets/cSocket.h"

#include <list>
#include <ostream>
#include <iostream>
#include <cassert>
#include <string>

class cChat_Server :
	public cChat
{
public:
	cChat_Server(cNetwork &network);
	virtual ~cChat_Server();
	
	void Process(int connections, const std::string& port);

private:
	void inConnect(cCommand& command, cSocket* socket);
	void inDisconnet(cCommand& command, cSocket* socket);
	void inRoomCreate(cCommand& command, cSocket* socket);
	void inRoomConnect(cCommand& command, cSocket* socket);
	void inRoomLeave(cCommand& command, cSocket* socket);
	void inMessage(cCommand& command, cSocket* socket);

private:
	cNetwork &m_network;

	// Map ID's to sockets;
	typedef std::map<cCommand::tUID, cSocket*> mapSockets;
	mapSockets m_mapSockets;
	cSocket* GetSocket(cCommand::tUID uid) const;
	cCommand::tUID GetUIDFromSocket(cSocket* in) const;
	void AddSocket(cCommand::tUID UID, cSocket* sock);
	void RemoveSocket(cCommand::tUID uid);
private:
	cChat_Server();
	cChat_Server(const cChat_Server&);
	cChat_Server operator=(const cChat_Server&);
};

