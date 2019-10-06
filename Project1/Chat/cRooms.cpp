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
	m_uids.push_back(UID);
}

void cRoom::RemoveUID(cCommand::tUID UID)
{
	listUIDs::iterator it;

	// Fetch the iterator of element with value 'the'
	it = std::find(m_uids.begin(), m_uids.end(), UID);

	// Check if iterator points to end or not
	if (it != m_uids.end())
		m_uids.erase(it);
}

bool cRoom::HasUID(cCommand::tUID UID) const
{
	// Fetch the iterator of element with value 'the'
	auto it = std::find(m_uids.begin(), m_uids.end(), UID);

	// Check if iterator points to end or not
	return (it != m_uids.end());
}

cRoom::listUIDs& cRoom::GetListUIDs()
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

cRoom& cRooms::AddRoom(cCommand::tRoom room, const std::string& name)
{
	cRoom* newRoom = new cRoom(room, name);
	m_mapRooms[room] = newRoom;
	return *m_mapRooms[room];
}

cRoom& cRooms::GetRoom(cCommand::tRoom room)
{
	return *m_mapRooms[room];
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
