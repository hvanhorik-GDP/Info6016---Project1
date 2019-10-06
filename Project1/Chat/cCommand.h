#pragma once
#include <limits>
#include <iostream>
#include <ostream>

class cCommand
{
public:

	enum eCommand
	{
		eNop,
		eConnect,
		eConnected,
		eDisconnect,
		eDisconnected,
		eRoomCreate,
		eRoomCreated,
		eRoomConnect,
		eRoomConnected,
		eMessage,
		eMaxMessage = eMessage,
		eUnknown
	};
	typedef unsigned short tUID;
	typedef unsigned short tRoom;
	static const tUID tUID_Unknown = 0;			// System UID	
	static const tUID tUID_System = 1;			// System UID	
	static const tRoom tRoom_Unknown = 0;		// System Room ID (No room)

	cCommand(
		eCommand command,
		tUID uid, 
		tRoom room, 
		const char* message,
		unsigned short size);
	cCommand(
		const char* rawBuffer,
		unsigned short size);

	cCommand();
	~cCommand();

	tUID GetMaxID();
	tRoom GetMaxRooms();
	static unsigned short GetMaxRawLength();
	static unsigned short GetMaxMessageLength();
	static unsigned short GetHeaderLength();

	cCommand::eCommand GetCommand() const;
	cCommand::tUID GetUID() const;
	cCommand::tRoom GetRoom() const;
	const char* GetRawMessage() const;
	unsigned short GetRawLength() const;
	unsigned short GetMessageLength() const;
	const char* GetTheMessage() const;

	friend std::ostream& operator<<(std::ostream& stream, const cCommand& info);

private:
	void ClearRawBuffer();
	static void PackBE(char* buffer, unsigned short value);
	static unsigned short UnpackBE(char* buffer);

	// Some static variables
	static const std::size_t tBufferLength = 1024;

	// Format of the raw buffer - Note it will be packed/unpacked correctly
	// before sending to the network - This is just so we get offsets into the 
	// raw buffer
	struct sRawBuffer
	{
		char			m_TotalLength[2];		// 2 bytes for a command
		char			m_Command;				// 1 bytes for command (even though its an enum)
		char			m_uid[2];				// 2 bytes for UID
		char			m_room[2];				// 2 bytes for a room
		char			m_messageLength[2];		// 2 bytes for message length
		static const std::size_t tHeaderLength = 2 + 1 + 2 + 2 + 2;
		char			m_message[tBufferLength - tHeaderLength-1];	// Less one for pad
		char			m_pad;					// Zero terminate padding
	};

	sRawBuffer* m_RawBuffer;

private:
	cCommand(const cCommand& in);
	cCommand operator = (const  cCommand& in);

};

