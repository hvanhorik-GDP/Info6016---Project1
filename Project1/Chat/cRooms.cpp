#include "cRooms.h"
#include <algorithm>

cRoom::cRoom(cCommand::tRoom roomID, const std::string& name)
	: m_roomID(roomID)
	, m_roomName(name)
{
}

cRoom::~cRoom()
{
}

void cRoom::AddUID(cCommand::tUID UID)
{
	m_uids[UID] = GetName();
}

void cRoom::RemoveUID(cCommand::tUID UID)
{
	if (m_uids.find(UID) != m_uids.end())
		m_uids.erase(UID);
}

bool cRoom::HasUID(cCommand::tUID UID) const
{
	return (m_uids.find(UID) != m_uids.end());
}

cRoom::mapUIDs& cRoom::GetMapUIDs()
{
	return m_uids;
}

const std::string& cRoom::GetName() const
{
	return m_roomName;
}

cRooms::cRooms()
{
}

cRooms::~cRooms()
{
	for (auto rooms : m_mapRooms)
	{
		cRoom* rm = rooms.second;
		delete rm;
	}
}

cRoom* cRooms::AddRoom(cCommand::tRoom room, const std::string& name)
{
	cRoom* newRoom = new cRoom(room, name);
	m_mapRooms[room] = newRoom;
	return m_mapRooms[room];
}

cRoom* cRooms::GetRoom(cCommand::tRoom room)
{
	return m_mapRooms[room];
}

void cRooms::RemoveRoom(cCommand::tRoom room)
{
	auto it = m_mapRooms.find(room);
	if ( it != m_mapRooms.end())
	{
		cRoom* rm = it->second;
		delete rm;
		m_mapRooms.erase(room);
	}
}

const cRooms::mapRooms& cRooms::GetMapRooms() const
{
	return m_mapRooms;
}
