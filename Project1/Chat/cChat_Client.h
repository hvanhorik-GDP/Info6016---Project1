#pragma once
#include "cChat.h"
#include "../Sockets/cNetwork.h"
#include <string>
#include <vector>

class cChat_Client :
	public cChat
{
public:
	typedef std::vector<std::string> vecTokens;

	cChat_Client(cNetwork &network);
	virtual ~cChat_Client();
	
	void Process(const std::string& port);

private:
	void cmdProcess(const char* line);
	void cmdConnect(vecTokens tokens);
	void cmdDisconnect(vecTokens tokens);
	void cmdCreate(vecTokens tokens);
	void cmdRoom(vecTokens tokens);
	void cmdLeave(vecTokens tokens);
	void cmdWait(vecTokens tokens);
	void cmdMessage(vecTokens tokens, const char* line);
	void cmdExit(vecTokens tokens);
	void cmdRooms(vecTokens tokens);
	void cmdUsers(vecTokens tokens);


	void inCheckForResponse();
	void inConnected(cCommand& command);
	void inDisconneted(cCommand& command);
	void inRoomCreated(cCommand& command);
	void inRoomConnected(cCommand& command);
	void inRoomLeft(cCommand& command);
	void inMessage(cCommand& command);

private:
	cNetwork &m_network;
	bool m_connected = false;
	bool m_waitingForConnect = false;
	bool m_wedidsomething = false;
	bool m_blockingMessage = false;			// Debug
	bool m_exiting = false;
	bool m_reprompt = true;
	std::string m_helpString;

	// Current UID
	cCommand::tUID m_UID = cCommand::tUID_Unknown;
	std::string m_connectedName;
	// Current Room
	cCommand::tRoom m_Room = cCommand::tRoom_Unknown;
	std::string m_connectedRoom;
	cSocket* m_socket = NULL;
private:
	cChat_Client();
	cChat_Client(const cChat_Client&);
	cChat_Client operator=(const cChat_Client&);
};

