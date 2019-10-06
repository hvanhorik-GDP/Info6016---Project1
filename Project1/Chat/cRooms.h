#pragma once
#include "cCommand.h"

#include <map>
#include <list>

class cRoom
{
public:
	typedef std::list<cCommand::tUID> listUIDs;

	cRoom(cCommand::tRoom roomID, const std::string& name);
	~cRoom();
	void AddUID(cCommand::tUID UID);
	void RemoveUID(cCommand::tUID UID);
	bool HasUID(cCommand::tUID UID) const;
	cRoom::listUIDs& GetListUIDs();
	const std::string& GetName() const;

private:
	cRoom();
	cCommand::tRoom m_roomID;
	std::string m_roomName;
	cRoom::listUIDs m_uids;
};

class cRooms
{
public:
	typedef std::map<cCommand::tRoom, cRoom*> mapRooms;

	cRooms();
	~cRooms();

	cRoom& AddRoom(cCommand::tRoom room, const std::string& name);
	cRoom& GetRoom(cCommand::tRoom room);
	void RemoveRoom(cCommand::tRoom room);
	const cRooms::mapRooms& GetMapRooms() const;
private:
	// Map ID's to Rooms
	cRooms::mapRooms m_mapRooms;
};

