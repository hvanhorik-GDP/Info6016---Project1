#include "cChat_Server.h"

using namespace std;

cChat_Server::cChat_Server(cNetwork& network)
	: m_network(network)
{

}

cChat_Server::~cChat_Server()
{
}


void cChat_Server::inConnect(cCommand& command, cSocket *socket)
{
	auto uid = command.GetUID();				// The new User UID
	auto room = 0;								// No Room
	auto message = command.GetTheMessage();		// There Name
	auto size = command.GetMessageLength();		// and it's length

	// Add the user to the global list
	uid = GetNextFreeUID();
	std::cout << "Adding UID: " << uid << " - " << message << endl;
	AddUID(uid, message);		// Add the new user
	AddSocket(uid, socket);
	cCommand response(cCommand::eConnected, uid, room, message, size);

	// Broadcast new Connect to all users
	for (auto user : m_mapSockets)
	{
		Sleep(50);
		auto sock = user.second;
		cout << "Sending response:" << " to: " << sock->GetID() <<endl;
		cout << response;
		sock->Send(response.GetRawMessage(), response.GetRawLength());
	}

	// Broadcast everyones name
	for (auto user : m_All_UIDs)
	{
		if (user.first != uid)
		{
			Sleep(50);
			auto theUID = user.first;
			auto theName = user.second;
			auto theSize = (unsigned short)theName.size();
			cCommand theResponse(cCommand::eConnected, theUID, 0, theName.c_str(), theSize);
			cout << "Broadcasting name:" << socket->GetID() << endl;
			cout << theResponse;
			socket->Send(theResponse.GetRawMessage(), theResponse.GetRawLength());
		}
	}
	auto rooms = m_Rooms.GetMapRooms();

	// Broadcast available rooms back to the new user.
	for (auto it : rooms)
	{
		Sleep(50);
		auto rm = it.first;
		auto rmname = it.second->GetName();
		auto sz = (short)rmname.size();
		cCommand rmResponse(cCommand::eRoomCreated, uid, rm, rmname.c_str(), sz);
		cout << "Sending Rooms: " << socket->GetID() << endl;
		cout << rmResponse;
		socket->Send(rmResponse.GetRawMessage(), rmResponse.GetRawLength());
	}
}

void cChat_Server::inDisconnet(cCommand& command, cSocket* socket)
{
	auto uid = command.GetUID();				// The new User UID
	auto room = 0;								// No Room
	auto message = command.GetTheMessage();		// There Name
	auto size = command.GetMessageLength();		// and it's length

	cout << "RemoveUID: " << uid << " - " << message << endl;
	RemoveUID(uid);						// Remove the user
	cCommand response(cCommand::eDisconnected, uid, room, message, size);
	cout << "Sending disconnect:" << endl;
	cout << response;
	cout << "Responding to disconnect :" << endl << response;

	// Remove the user from all rooms
	auto rooms = m_Rooms.GetMapRooms();
	for (auto it : rooms)
	{
		auto rm = it.second;
		rm->RemoveUID(uid);
	}

	RemoveSocket(uid);					// No need for socket

	// Tell everyone else they has disconnected
	for (auto it : m_mapSockets)
	{
		Sleep(50);
		auto sock = it.second;
		sock->Send(response.GetRawMessage(), response.GetRawLength());
	}
}

void cChat_Server::inRoomCreate(cCommand& command, cSocket* socket)
{
	auto uid = command.GetUID();				// The new User UID
	auto room = command.GetRoom();				// Room
	auto message = command.GetTheMessage();		// There Name
	auto size = command.GetMessageLength();		// and it's length

	// Add the user to the global list
	cout << "Adding Room: " << room << " - " << message << endl;
	room = GetNextFreeRoom();
	AddRoom(room, message);					// Add the new user
	cCommand response(cCommand::eRoomCreated, uid, room, message, size);
	cout << "Sending Room Created:" << endl;
	cout << response;

	// Broadcast new room
	for (auto user : m_mapSockets)
	{
		Sleep(50);
		auto sock = user.second;
		sock->Send(response.GetRawMessage(), response.GetRawLength());
	}
}

void cChat_Server::inRoomConnect(cCommand& command, cSocket* socket)
{
	auto uid = command.GetUID();				// The new User UID
	auto room = command.GetRoom();				// Room
	auto message = command.GetTheMessage();		// There Name
	auto size = command.GetMessageLength();		// and it's length

	// Add the user to the global list
	cout << "Connecting to Room: " << room 
		 << " UID: " << uid 
		 << " Message: " << message << endl;

	auto theRoom = m_Rooms.GetRoom(room);
	theRoom->AddUID(uid);

	cCommand response(cCommand::eRoomConnected, uid, room, message, size);
	cout << "Sending Room connect:" << endl;
	cout << response;

	// Broadcast new person in the room
	auto users = theRoom->GetMapUIDs();
	for (auto user : users)
	{
		auto sock = GetSocket(user.first);
		sock->Send(response.GetRawMessage(), response.GetRawLength());
	}
}

void cChat_Server::inRoomLeave(cCommand& command, cSocket* socket)
{
	auto uid = command.GetUID();				// The new User UID
	auto room = command.GetRoom();				// Room
	auto message = command.GetTheMessage();		// There Name
	auto size = command.GetMessageLength();		// and it's length

	// Add the user to the global list
	cout << "leaving to Room: " << room
		<< " UID: " << uid
		<< " Message: " << message << endl;

	auto theRoom = m_Rooms.GetRoom(room);
	theRoom->RemoveUID(uid);

	cCommand response(cCommand::eRoomLeft, uid, room, message, size);
	cout << "Sending Room left:" << endl;
	cout << response;

	// Broadcast new person in the room
	auto users = theRoom->GetMapUIDs();
	for (auto user : users)
	{
		auto sock = GetSocket(user.first);
		sock->Send(response.GetRawMessage(), response.GetRawLength());
	}
}

void cChat_Server::inMessage(cCommand& command, cSocket* socket)
{
	auto uid = command.GetUID();				// The new User UID
	auto room = command.GetRoom();				// Room
	auto message = command.GetTheMessage();		// There Name
	auto size = command.GetMessageLength();		// and it's length

	// Add the user to the global list
	cout << "Mesaage from user: " << uid
		<< " UID: " << uid
		<< " Message: " << message << endl;

	auto theRoom = m_Rooms.GetRoom(room);

	// Broadcast new person in the room
	auto users = theRoom->GetMapUIDs();
	for (auto user : users)
	{
		// Only send it to others in the room
		if (user.first != uid)
		{
			auto sock = GetSocket(user.first);
			sock->Send(command.GetRawMessage(), command.GetRawLength());
		}
	}
}

cSocket* cChat_Server::GetSocket(cCommand::tUID uid) const
{
	auto id = m_mapSockets.find(uid);
	if (id != m_mapSockets.end())
		return id->second;
	return NULL;
}

cCommand::tUID cChat_Server::GetUIDFromSocket(cSocket* in) const
{
	for (auto uid : m_mapSockets)
	{
		if (uid.second == in)
			return uid.first;
	}
	return cCommand::tUID_Unknown;
}

void cChat_Server::AddSocket(cCommand::tUID UID, cSocket* sock)
{
	m_mapSockets[UID] = sock;
}

void cChat_Server::RemoveSocket(cCommand::tUID uid)
{
	auto id = m_mapSockets.find(uid);
	if (id != m_mapSockets.end())
		m_mapSockets.erase(uid);
}

void cChat_Server::Process( int connections, const std::string& port)
{
	auto listen = m_network.CreateListenSocket(connections);
	listen->GetInfo(port);
	listen->Create();
	listen->Bind();
	listen->Listen();
	listen->SetNonBlocking();

	size_t recBufferLength = cCommand::GetMaxRawLength();
	char* recBuffer = new char[recBufferLength];
	size_t recieved;

	bool wedidSomething = false;
	bool blockingmsg = false;				// Debug
	bool done = false;
	while (!done)
	{
		{
			// Check for new connection
			cSocket* newSock = listen->Accept();
			if (newSock)
			{
				newSock->SetNonBlocking();
				m_network.AddSocket(newSock);
				wedidSomething = true;			// No sleep
			}
		}

		auto sockets = m_network.GetMapSockets();
		// Check each of our sockets for new information
		for (auto it : sockets)
		{
			auto sock = it.second;
			recieved = sock->Recieve(recBuffer, recBufferLength);
			if (recieved == 0)
			{
				cout << "Connection closed." << endl;
				auto uid = GetUIDFromSocket(sock);

				// Dummy disconnect message
				cCommand dummy(cCommand::eDisconnect, uid, 0, "", 0);
				inDisconnet(dummy, sock);
				cout << "Recieved an error on socket" << sock->GetID() << endl;
				m_network.RemoveSocket(sock);
				break;				// break out of for loop since map will be messed
			}
			else if (recieved == WSAEWOULDBLOCK)
			{
				if (!blockingmsg)
					cout << "We are blocking" << endl;
				blockingmsg = true;
				continue;
			}
			else if (recieved > cCommand::GetMaxRawLength())
			{
				auto uid = GetUIDFromSocket(sock);

				// Dummy disconnect message
				cCommand dummy(cCommand::eDisconnect, uid, 0, "", 0);
				inDisconnet(dummy, sock);
				cout << "Recieved an error on socket" << sock->GetID() << endl;
				m_network.RemoveSocket(sock);
				break;				// break out of for loop since map will be messed
			}
			wedidSomething = true;		// No sleep
			blockingmsg = false;
			cCommand command(recBuffer, (unsigned short)recieved);
			cout << command;
			auto cmd = command.GetCommand();
			switch (cmd)
			{
			case cCommand::eConnect:
			{
				inConnect(command, sock);
				break;
			}
			case cCommand::eDisconnect:
			{
				inDisconnet(command, sock);
				break;
			}
			case cCommand::eRoomCreate:
			{
				inRoomCreate(command, sock);
				break;
			}
			case cCommand::eRoomConnect:
			{
				inRoomConnect(command, sock);
				break;
			}
			case cCommand::eRoomLeave:
			{
				inRoomLeave(command, sock);
				break;
			}
			case cCommand::eMessage:
			{
				inMessage(command, sock);
				break;
			}
			default:
			{
				cout << "unknown command: " << cmd << endl;
				cout << command;
			}
			}
		}
		if (!wedidSomething)
			Sleep(1);
	} // while (!done)
}