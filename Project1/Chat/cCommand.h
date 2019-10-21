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
		eRoomLeave,
		eRoomLeft,
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
		unsigned short size);				// Message length
	cCommand(
		const char* rawBuffer,
		unsigned long size);				// Raw buffer length

	cCommand();
	~cCommand();

	tUID GetMaxID();
	tRoom GetMaxRooms();
	static unsigned long GetMaxRawLength();
	static unsigned short GetMaxMessageLength();
	static unsigned short GetHeaderLength();

	cCommand::eCommand GetCommand() const;
	cCommand::tUID GetUID() const;
	cCommand::tRoom GetRoom() const;
	const char* GetRawMessage() const;
	unsigned long GetRawLength() const;
	unsigned short GetMessageLength() const;
	const char* GetTheMessage() const;

	friend std::ostream& operator<<(std::ostream& stream, const cCommand& info);

	static void PackuShortBE(char* buffer, unsigned short value);
	static unsigned short UnPackuShortBE(char* buffer);
	static void PackuLongBE(char* buffer, unsigned long value);
	static unsigned long UnPackuLongBE(char* buffer);

private:
	void ClearRawBuffer();

	// Some static variables
	static const std::size_t tBufferLength = 512;		// Make him work to check overflow

	// Format of the raw buffer - Note it will be packed/unpacked correctly
	// before sending to the network - This is just so we get offsets into the 
	// raw buffer
	struct sRawBuffer
	{
		char			m_TotalLength[4];		// 4 bytes for a command length
		char			m_Command;				// 1 bytes for command (even though its an enum)
		char			m_uid[2];				// 2 bytes for UID
		char			m_room[2];				// 2 bytes for a room
		char			m_messageLength[2];		// 2 bytes for message length
		static const std::size_t tHeaderLength = 4 + 1 + 2 + 2 + 2;
		char			m_message[tBufferLength - tHeaderLength];	// The message - total - header
		char			m_pad;					// Zero terminate padding
	};

	sRawBuffer* m_RawBuffer;

private:
	cCommand(const cCommand& in);
	cCommand operator = (const  cCommand& in);

};

std::ostream& operator<<(std::ostream& stream, cCommand::eCommand in);
