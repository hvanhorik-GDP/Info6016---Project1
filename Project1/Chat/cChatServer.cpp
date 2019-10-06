#include "cChatServer.h"

using namespace std;

cChatServer::cChatServer(cNetwork& network)
	: m_network(network)
{

}

cChatServer::~cChatServer()
{
}

void cChatServer::Process()
{

}

void cChatServer::inConnect(cCommand& command, cSocket &socket)
{
	auto uid = command.GetUID();				// The new User UID
	auto room = 0;								// No Room
	auto message = command.GetTheMessage();		// There Name
	auto size = command.GetMessageLength();		// and it's length

	// Add the user to the global list
	uid = GetNextFreeUID();
	cout << "Adding UID: " << uid << " - " << message << endl;
	AddUID(uid, message);		// Add the new user
	cCommand response(cCommand::eConnected, uid, room, message, size);

	// Broadcast new Connect to all users
	for (auto user : m_mapSockets)
	{
		auto sock = user.second;
		sock->Send(response.GetRawMessage(), response.GetRawLength());
	}
	// Broadcast everyones name

	for (auto user : m_All_UIDs)
	{
		if (user.first != uid)
		{
			auto theUID = user.first;
			auto theName = user.second;
			auto theSize = (unsigned short)theName.size();
			cCommand theResponse(cCommand::eConnected, theUID, 0, theName.c_str(), theSize);
			socket.Send(theResponse.GetRawMessage(), theResponse.GetRawLength());
		}
	}
	auto rooms = m_Rooms.GetMapRooms();

	// Broadcast available rooms back to the new user.
	for (auto it : rooms)
	{
		auto rm = it.first;
		auto rmname = it.second->GetName();
		auto sz = (short)rmname.size();
		cCommand rmResponse(cCommand::eRoomConnected, uid, rm, rmname.c_str(), sz);
		socket.Send(rmResponse.GetRawMessage(), rmResponse.GetRawLength());
	}
}

void cChatServer::inDisconnet(cCommand& command, cSocket& socket)
{
	auto uid = command.GetUID();				// The new User UID
	auto room = 0;								// No Room
	auto message = command.GetTheMessage();		// There Name
	auto size = command.GetMessageLength();		// and it's length

	cout << "RemoveUID: " << uid << " - " << message << endl;
	RemoveUID(uid);						// Remove the user
	cCommand response(cCommand::eDisconnected, uid, room, message, size);
	cout << "Responding to disconnect :" << endl << response;

	// Remove the user from all rooms
	auto rooms = m_Rooms.GetMapRooms();
	for (auto it : rooms)
	{
		auto rm = it.second;
		rm->RemoveUID(uid);
	}

	// Tell everyone he has disconnected
	for (auto it : m_mapSockets)
	{
		auto sock = it.second;
		sock->Send(response.GetRawMessage(), response.GetRawLength());
	}
}

void cChatServer::inRoomCreate(cCommand& command, cSocket& socket)
{
	auto uid = command.GetUID();				// The new User UID
	auto room = command.GetRoom();				// Room
	auto message = command.GetTheMessage();		// There Name
	auto size = command.GetMessageLength();		// and it's length

	// Add the user to the global list
	cout << "Adding Room: " << room << " - " << message << endl;
	room = GetNextFreeRoom();
	AddRoom(room, message);					// Add the new user
	cCommand response(cCommand::eRoomCreate, uid, room, message, size);

	// Broadcast new room
	for (auto user : m_mapSockets)
	{
		auto sock = user.second;
		sock->Send(response.GetRawMessage(), response.GetRawLength());
	}
}

void cChatServer::inRoomConnect(cCommand& command, cSocket& socket)
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
	theRoom.AddUID(uid);

	cCommand response(cCommand::eRoomConnected, uid, room, message, size);

	// Broadcast new person in the room
	auto rooms = m_Rooms.GetMapRooms();
	for (auto it : rooms)
	{
		auto sock = GetSocket(it.first);
		sock->Send(response.GetRawMessage(), response.GetRawLength());
	}
}

void cChatServer::inMessage(cCommand& command, cSocket& socket)
{

}

cSocket* cChatServer::GetSocket(cCommand::tUID uid) const
{
	auto id = m_mapSockets.find(uid);
	if (id != m_mapSockets.end())
		return id->second;
	return NULL;
}

cCommand::tUID cChatServer::GetUIDFromSocket(cSocket* in) const
{
	for (auto uid : m_mapSockets)
	{
		if (uid.second == in)
			return uid.first;
	}
	return cCommand::tUID_Unknown;
}

void cChatServer::AddSocket(cCommand::tUID UID, cSocket* sock)
{
	m_mapSockets[UID] = sock;
}

void cChatServer::RemoveSocket(cCommand::tUID uid)
{
	auto id = m_mapSockets.find(uid);
	if (id != m_mapSockets.end())
		m_mapSockets.erase(uid);
}
//
////mapUIDRoom m_All_mapUIDRoom;
//cCommand::tRoom cChatServer::GetRoomFromUID(cCommand::tUID uid) const
//{
//	for (auto room : m_mapUIDRoom)
//	{
//		if (room.second == uid)
//			return room.first;
//	}
//	return cCommand::tRoom_Unknown;
//}
//
//cCommand::tUID cChatServer::GetUIDFromRoom(cCommand::tRoom room) const
//{
//	auto id = m_mapUIDRoom.find(room);
//	if (id != m_mapUIDRoom.end())
//		return id->second;
//	return cCommand::tRoom_Unknown;
//}
//
//void cChatServer::AddUIDToRoom(cCommand::tRoom room, cCommand::tUID UID)
//{
//	m_mapUIDRoom[room] = UID;
//}
//
//void cChatServer::RemoveUIDFromRoom(cCommand::tRoom room)
//{
//	auto id = m_mapUIDRoom.find(room);
//	if (id != m_mapUIDRoom.end())
//		m_All_mapUIDRoom.erase(room);
//}
