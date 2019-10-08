#pragma once
#include "cCommand.h"

#include <map>

class cRoom
{
public:
	typedef std::map<cCommand::tUID, std::string> mapUIDs;

	cRoom(cCommand::tRoom roomID, const std::string& name);
	~cRoom();
	void AddUID(cCommand::tUID UID);
	void RemoveUID(cCommand::tUID UID);
	bool HasUID(cCommand::tUID UID) const;
	cRoom::mapUIDs& GetMapUIDs();
	const std::string& GetName() const;

private:
	cRoom();
	cRoom(const cRoom &);
	cRoom& operator=(const cRoom&);
	cCommand::tRoom m_roomID;
	std::string m_roomName;
	cRoom::mapUIDs m_uids;
};

class cRooms
{
public:
	typedef std::map<cCommand::tRoom, cRoom*> mapRooms;

	cRooms();
	~cRooms();

	cRoom* AddRoom(cCommand::tRoom room, const std::string& name);
	cRoom* GetRoom(cCommand::tRoom room);
	void RemoveRoom(cCommand::tRoom room);
	const cRooms::mapRooms& GetMapRooms() const;
private:
	cRooms(const cRooms&);
	cRooms& operator=(const cRooms&);

	// Map ID's to Rooms
	cRooms::mapRooms m_mapRooms;
};

