#include "cCommand.h"
#include <limits>
#include <cassert>
#include <cstdio>
#include <cstring>

extern bool gDebug;				// External debug flag

cCommand::cCommand(
	eCommand command, 
	tUID uid, 
	tRoom room,
	const char* message,
	unsigned short size)					// Message length
	:  m_RawBuffer(new sRawBuffer())
{
	if (size > GetMaxMessageLength())
	{
		// This should have been handled
		assert(false);
		std::cout << "command buffer overflow... truncating" << size << std::endl;
		size = GetMaxMessageLength();
	}
	ClearRawBuffer();
	m_RawBuffer->m_Command = command;
	PackuShortBE(m_RawBuffer->m_uid, uid);
	PackuShortBE(m_RawBuffer->m_room, room);
	PackuShortBE(m_RawBuffer->m_messageLength, size);
	std::memcpy(m_RawBuffer->m_message, message, size);
	PackuLongBE(m_RawBuffer->m_TotalLength, GetHeaderLength() + size);
}

cCommand::cCommand(
	const char* rawBuffer,
	unsigned long size)							// Raw buffer length
	: m_RawBuffer(new sRawBuffer())
{
	if (size > GetMaxRawLength())
	{
		// This should have been handled
		assert(false);
		std::cout << "command buffer overflow... truncating" << size << std::endl;
		size = GetMaxRawLength();
	}
	memcpy(m_RawBuffer, rawBuffer, size);
	// Lets do some checks on the buffer;
	unsigned long sizeBuffer = UnPackuLongBE(m_RawBuffer->m_TotalLength);
	if (sizeBuffer > GetMaxRawLength())
	{
		assert(false);
		std::cout << "command buffer overflow... truncating" << sizeBuffer << std::endl;
		size = GetMaxRawLength();
	}
	assert(size <= GetMaxRawLength());
	eCommand command = (eCommand)m_RawBuffer->m_Command;
	assert(command <= eMaxMessage);
	tUID uid = UnPackuShortBE(m_RawBuffer->m_uid);
	tRoom room = UnPackuShortBE(m_RawBuffer->m_room);
	unsigned short messageLength = UnPackuShortBE(m_RawBuffer->m_messageLength);
	if (messageLength > GetMaxMessageLength())
	{
		assert(false);
		std::cout << "command buffer overflow... truncating" << messageLength << std::endl;
		messageLength = GetMaxMessageLength();
	}
	assert(messageLength <= GetMaxMessageLength());
}

cCommand::cCommand()
	:  m_RawBuffer(new sRawBuffer())
{
	ClearRawBuffer();		// This initializes everything properly
	PackuShortBE(m_RawBuffer->m_TotalLength, GetHeaderLength() + 2); // Just a NOP with no data
}

cCommand::~cCommand() 
{
	delete[] m_RawBuffer;
}

unsigned short cCommand::GetHeaderLength()
{
	return sRawBuffer::tHeaderLength;
}


const char* cCommand::GetRawMessage() const
{
	assert(m_RawBuffer != NULL);
	return (const char*)m_RawBuffer;
}

unsigned long cCommand::GetRawLength() const
{
	auto size = UnPackuLongBE(m_RawBuffer->m_TotalLength);
	// Check for overflow
	if (size > GetMaxRawLength())
	{
		std::cout << "command buffer overflow... truncating" << size << std::endl;
		size = GetMaxRawLength();
	}

	return size;
}

const char* cCommand::GetTheMessage() const
{
	assert(m_RawBuffer);
	return m_RawBuffer->m_message; // Offset into the buffer
}

unsigned short cCommand::GetMessageLength() const
{
	auto size = UnPackuShortBE(m_RawBuffer->m_messageLength);
	// Check for overflow
	if (size > GetMaxMessageLength())
	{
		std::cout << "command buffer overflow... truncating" << size << std::endl;
		size = GetMaxMessageLength();
	}
	return size;
}

cCommand::eCommand cCommand::GetCommand() const
{
	return (eCommand)m_RawBuffer->m_Command;
}

cCommand::tUID cCommand::GetUID() const
{
	return UnPackuShortBE(m_RawBuffer->m_uid);
}

cCommand::tRoom cCommand::GetRoom() const
{
	return UnPackuShortBE(m_RawBuffer->m_room);
}

// Helper static methods

void cCommand::PackuShortBE(char* buffer, unsigned short value)
{
	buffer[0] = value >> 8;	// 00000000 00000000 [01010101] 11110000
	buffer[1] = value >> 0;	// 00000000 00000000 01010101 [11110000]
}


unsigned short cCommand::UnPackuShortBE(char* buffer)
{
	unsigned short value = 0;
	value |= buffer[0] << 8;		// 11110000 01010101 [00000000] 00000000
	value |= buffer[1] << 0;		// 11110000 01010101 00000000 [00000000]
	return value;
}

void cCommand::PackuLongBE(char* buffer, unsigned long value)
{
	buffer[0] = char(value >> 24);
	buffer[1] = char(value >> 16);
	buffer[2] = char(value >> 8);	// 00000000 00000000 [01010101] 11110000
	buffer[3] = char(value >> 0);	// 00000000 00000000 01010101 [11110000]

}
unsigned long cCommand::UnPackuLongBE(char* buffer)
{
	unsigned short value = 0;
	value |= buffer[0] << 24;		// 11110000 01010101 [00000000] 00000000
	value |= buffer[1] << 16;		// 11110000 01010101 00000000 [00000000]
	value |= buffer[2] << 8;		// 11110000 01010101 [00000000] 00000000
	value |= buffer[3] << 0;		// 11110000 01010101 00000000 [00000000]
	return value;
}

unsigned short cCommand::GetMaxMessageLength()
{
	return tBufferLength - GetHeaderLength();
}

unsigned long cCommand::GetMaxRawLength()
{
	return tBufferLength;
}


unsigned short cCommand::GetMaxID()
{
	return std::numeric_limits<tUID>::max() - 1;
}

unsigned short cCommand::GetMaxRooms()
{
	return std::numeric_limits<tRoom>::max() - 1;
}


void cCommand::ClearRawBuffer()
{
	assert(m_RawBuffer);
	// Setting to zero is the defactor correct thing to do since all
	// lengths, IDs, commands etc will be zero.
	std::memset(m_RawBuffer, 0, sizeof(m_RawBuffer));
}

std::ostream& operator<<(std::ostream& stream, const cCommand& info)
{
	if (gDebug)
	{
		stream << "Command:" << std::endl;
		stream << "    Total Length:   " << info.GetRawLength() << std::endl;
		stream << "    Command:        " << info.GetCommand() << std::endl;
		stream << "    UID:            " << info.GetUID() << std::endl;
		stream << "    Room:           " << info.GetRoom() << std::endl;
		stream << "    Message Length: " << info.GetMessageLength() << std::endl;
		stream << "    Message:        " << info.GetTheMessage() << std::endl;
	}
	return stream;
}

std::ostream& operator<<(std::ostream& stream, cCommand::eCommand in)
{
	if (gDebug)
	{
		stream << (int)in << " (";
		switch (in)
		{
		case cCommand::eNop:
			stream << "eNop";
			break;
		case cCommand::eConnect:
			stream << "eConnect";
			break;
		case cCommand::eConnected:
			stream << "eConnected";
			break;
		case cCommand::eDisconnect:
			stream << "eDisconnect";
			break;
		case cCommand::eDisconnected:
			stream << "eDisconnected";
			break;
		case cCommand::eRoomCreate:
			stream << "eRoomCreate";
			break;
		case cCommand::eRoomCreated:
			stream << "eRoomCreated";
			break;
		case cCommand::eRoomConnect:
			stream << "eRoomConnect";
			break;
		case cCommand::eRoomConnected:
			stream << "eRoomConnected";
			break;
		case cCommand::eMessage:
			stream << "eMessage";
			break;
		case cCommand::eUnknown:
		default:
			stream << "eUnknown";
			break;
		}
		stream << ")";
	}
	return stream;
}