#pragma once

#include <map>
#include <string>
#include "cCommand.h"
#include "cRooms.h"

class cChat
{
public:
	// Map ID's to strings;
	typedef std::map<cCommand::tUID, std::string> mapUID;
	// Map strings to commands
	typedef std::map<cCommand::eCommand, std::string> mapCommands;

	cChat();
	virtual ~cChat();

	const std::string& GetUIDName(cCommand::tUID) const;
	cCommand::tUID GetUID(std::string& name) const;
	void AddUID(cCommand::tUID UID, const std::string& name);
	void RemoveUID(cCommand::tUID);
	cCommand::tUID GetNextFreeUID();

	const std::string& GetRoomName(cCommand::tRoom room) const;
	cCommand::tRoom GetRoom(std::string & name) const;
	void AddRoom(cCommand::tRoom room, const std::string& name);
	void RemoveRoom(cCommand::tRoom room);
	cCommand::tRoom GetNextFreeRoom();

	static const std::string& GetCommandName(cCommand::eCommand in);
	static cCommand::eCommand GetCommand(const std::string& name);
	static void AddCommand(cCommand::eCommand id, const std::string& command);

protected:
	mapUID m_All_UIDs;
	cCommand::tUID m_nextUID = cCommand::tUID_System + 1;
	cRooms m_Rooms;
	cCommand::tRoom m_nextRoom = 1;
	static mapCommands m_All_Commands;
private:
	cChat(const cChat&);
	cChat& operator=(const cChat&);
};

// Commands are:
// Connect "Name"			-- Connect to the system with your name
// Disconnect				-- Disconnect from the system
// Create "Room"			-- Create a new chat room
// Switch "Room"			-- Switch to talking on that chat room
// Leave "Room"				-- Leave a chat room
// Wait "sec"				-- delay seconds
// Message "text"			-- send a message to the connected room
// Exit/End					-- Leave
// "text"					-- send text to current room (It will interecept Switch/Leave)
