#pragma once
#include "cChat.h"
#include "../Sockets/cNetwork.h"
#include "cCommand.h"
#include "../Sockets/cSocket.h"

#include <list>
#include <ostream>
#include <iostream>
#include <cassert>

class cChatServer :
	public cChat
{
public:
	cChatServer(cNetwork &network);
	virtual ~cChatServer();
	
	void Process();

private:
	void inConnect(cCommand& command, cSocket &socket);
	void inDisconnet(cCommand& command, cSocket& socket);
	void inRoomCreate(cCommand& command, cSocket& socket);
	void inRoomConnect(cCommand& command, cSocket& socket);
	void inMessage(cCommand& command, cSocket& socket);

	cChatServer();
	cChatServer(const cChatServer&);
	cChatServer operator=(const cChatServer&);
private:
	cNetwork m_network;

	struct cRoom
	{
		cCommand::tRoom m_roomID;
		std::string m_roomName;
		std::list<cCommand::tUID> m_uids;
	};

	//// Map ID's to Rooms
	//typedef std::map<cCommand::tRoom, cCommand::tUID > mapUIDRoom;
	//mapUIDRoom m_mapUIDRoom;
	//cCommand::tUID GetUIDFromRoom(cCommand::tRoom room) const;
	//cCommand::tRoom GetRoomFromUID(cCommand::tUID uid) const;
	//void AddUIDToRoom( cCommand::tRoom room, cCommand::tUID UID);
	//void RemoveUIDFromRoom(cCommand::tRoom room);

	// Map ID's to sockets;
	typedef std::map<cCommand::tUID, cSocket*> mapSockets;
	mapSockets m_mapSockets;
	cSocket* GetSocket(cCommand::tUID uid) const;
	cCommand::tUID GetUIDFromSocket(cSocket* in) const;
	void AddSocket(cCommand::tUID UID, cSocket* sock);
	void RemoveSocket(cCommand::tUID uid);
};

