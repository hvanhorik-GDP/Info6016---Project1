#include "cChat.h"

cChat::mapCommands cChat::m_All_Commands;

cChat::cChat()
{
	if (m_All_Commands.size() == 0)
	{
		AddCommand(cCommand::eNop, "nop");
		AddCommand(cCommand::eConnect, "connect");
		AddCommand(cCommand::eConnected, "connect");
		AddCommand(cCommand::eDisconnect, "Disconnect");
		AddCommand(cCommand::eDisconnected, "disconnected");
		AddCommand(cCommand::eRoomCreate, "roomcreate");
		AddCommand(cCommand::eRoomCreated, "roomcreated");
		AddCommand(cCommand::eRoomConnect, "roomconnect");
		AddCommand(cCommand::eRoomConnected, "roomconnected");
		AddCommand(cCommand::eMessage, "roommessagesend");
		AddCommand(cCommand::eUnknown, "unknown");
	}
}

cChat::~cChat()
{

}

const std::string& cChat::GetUIDName(cCommand::tUID in) const
{
	auto id = m_All_UIDs.find(in);
	if ( id != m_All_UIDs.end())
		return id->second;
	static std::string unknown("Unknown_UID");
	return unknown;
}

cCommand::tUID cChat::GetUID(std::string& name) const
{
	for (auto uid : m_All_UIDs)
	{
		if (uid.second == name)
			return uid.first;
	}
	return cCommand::tUID_Unknown;
}

void cChat::AddUID(cCommand::tUID UID, const std::string& name)
{
	m_All_UIDs[UID] = name;
}

void cChat::RemoveUID(cCommand::tUID uid)
{
	auto id = m_All_UIDs.find(uid);
	if (id != m_All_UIDs.end())
		m_All_UIDs.erase(uid);
}

const std::string& cChat::GetRoomName(cCommand::tRoom room) const
{
	auto rooms = m_Rooms.GetMapRooms();
	auto id = rooms.find(room);
	if (id != rooms.end())
		return id->second->GetName();
	static std::string unknown("Unknown_Room");
	return unknown;
}

cCommand::tUID cChat::GetNextFreeUID()
{
	// Buggy since it will overflow eventually
	m_nextUID++;
	return m_nextUID;
}

cCommand::tRoom cChat::GetNextFreeRoom()
{
	// Buggy since it will overflow eventually
	m_nextRoom++;
	return m_nextRoom;
}

cCommand::tRoom cChat::GetRoom(std::string& name) const
{
	auto rooms = m_Rooms.GetMapRooms();

	for (auto room : rooms)
	{
		if (room.second->GetName() == name)
			return room.first;
	}
	return cCommand::tRoom_Unknown;
}

void cChat::AddRoom(cCommand::tRoom room, const std::string& name)
{
	m_Rooms.AddRoom(room, name);
}

void cChat::RemoveRoom(cCommand::tRoom room)
{
	m_Rooms.RemoveRoom(room);
}

const std::string& cChat::GetCommandName(cCommand::eCommand in)
{
	auto id = m_All_Commands.find(in);
	if (id != m_All_Commands.end())
		return id->second;
	static std::string unknown("Unknown_Command");
	return unknown;
}

cCommand::eCommand cChat::GetCommand(const std::string& name)
{
	for (auto command : m_All_Commands)
	{
		if (command.second == name)
			return command.first;
	}
	return cCommand::eUnknown;
}

void cChat::AddCommand(cCommand::eCommand id, const std::string& command)
{
	m_All_Commands[id] = command;
}
