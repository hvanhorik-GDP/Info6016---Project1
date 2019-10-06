#include "cCommand.h"
#include <limits>
#include <cassert>
#include <cstdio>
#include <cstring>

cCommand::cCommand(
	eCommand command, 
	tUID uid, 
	tRoom room,
	const char* message,
	unsigned short size)
	:  m_RawBuffer(new sRawBuffer())
{
	assert(size <= GetMaxMessageLength());
	ClearRawBuffer();
	m_RawBuffer->m_Command = command;
	PackBE(m_RawBuffer->m_uid, uid);
	PackBE(m_RawBuffer->m_room, room);
	PackBE(m_RawBuffer->m_messageLength, size);
	std::memcpy(m_RawBuffer->m_message, message, size);
	PackBE(m_RawBuffer->m_TotalLength, GetHeaderLength() + 2 + size);
}

cCommand::cCommand(
	const char* rawBuffer,
	unsigned short size)
	: m_RawBuffer(new sRawBuffer())
{
	assert(size <= GetMaxRawLength());
	memcpy(m_RawBuffer, rawBuffer, size);
	// Lets do some checks on the buffer;
	unsigned short sizeBuffer = UnpackBE(m_RawBuffer->m_TotalLength);
	assert(size <= GetMaxRawLength());
	eCommand command = (eCommand)m_RawBuffer->m_Command;
	assert(command <= eMaxMessage);
	tUID uid = UnpackBE(m_RawBuffer->m_uid);
	tRoom room = UnpackBE(m_RawBuffer->m_room);
	unsigned short messageLength = UnpackBE(m_RawBuffer->m_messageLength);
	assert(messageLength <= GetMaxMessageLength());
}

cCommand::cCommand()
	:  m_RawBuffer(new sRawBuffer())
{
	ClearRawBuffer();		// This initializes everything properly
	PackBE(m_RawBuffer->m_TotalLength, GetHeaderLength() + 2); // Just a NOP with no data
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

unsigned short cCommand::GetRawLength() const
{
	return UnpackBE(m_RawBuffer->m_TotalLength) + 2;;
}

const char* cCommand::GetTheMessage() const
{
	assert(m_RawBuffer);
	return m_RawBuffer->m_message; // Offset into the buffer
}

unsigned short cCommand::GetMessageLength() const
{
	return UnpackBE(m_RawBuffer->m_messageLength);
}

cCommand::eCommand cCommand::GetCommand() const
{
	return (eCommand)m_RawBuffer->m_Command;
}

cCommand::tUID cCommand::GetUID() const
{
	return UnpackBE(m_RawBuffer->m_uid);
}

cCommand::tRoom cCommand::GetRoom() const
{
	return UnpackBE(m_RawBuffer->m_room);
}

// Helper static methods

void cCommand::PackBE(char* buffer, unsigned short value)
{
	buffer[0] = value >> 8;	// 00000000 00000000 [01010101] 11110000
	buffer[1] = value >> 0;	// 00000000 00000000 01010101 [11110000]
}


unsigned short cCommand::GetMaxMessageLength()
{
	return tBufferLength - GetHeaderLength();
}

unsigned short cCommand::GetMaxRawLength()
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

unsigned short cCommand::UnpackBE(char* buffer)
{
	unsigned short value = 0;
	value |= buffer[0] << 8;		// 11110000 01010101 [00000000] 00000000
	value |= buffer[1] << 0;		// 11110000 01010101 00000000 [00000000]
	return value;
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
	stream << "Command:" << std::endl;
	stream << "    Total Length:   " << info.GetRawLength() << std::endl;
	stream << "    Command:        " << info.GetCommand() << std::endl;
	stream << "    UID:            " << info.GetUID() << std::endl;
	stream << "    Room:           " << info.GetRoom() << std::endl;
	stream << "    Message Length: " << info.GetMessageLength() << std::endl;
	stream << "    Message:        " << info.GetTheMessage() << std::endl;
	return stream;
}
