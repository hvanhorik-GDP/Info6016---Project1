#include "cChat_Client.h"
#include <algorithm>
#include <vector>
#include <sstream>
#include <iostream>
#include <cassert>
#include <conio.h>

using namespace std;

#define ESCAPE 27
#define BACKSPACE 8
#define RETURN 13

extern bool gDebug;

void Tokens(const std::string& in, cChat_Client::vecTokens& out, const string& separators)
{
	std::string temp = in;
	for (size_t i = 1; i < separators.size(); ++i)
	{
		std::replace(temp.begin(), temp.end(), separators[i], separators[0]);
	}

	stringstream check(temp);

	string intermediate;

	// Tokenizing w.r.t. space ' ' 
	while (getline(check, intermediate, separators[0]))
	{
		if (intermediate.size() != 0)
			out.push_back(intermediate);
	}
}

char asciitolower(char in) {
	if (in <= 'Z' && in >= 'A')
		return in - ('Z' - 'z');
	return in;
}

void ToLower(std::string& data)
{

	std::transform(data.begin(), data.end(), data.begin(), asciitolower);
}

cChat_Client::cChat_Client(cNetwork& network)
	:	m_network(network)
{
}

cChat_Client::~cChat_Client()
{}

void cChat_Client::Process(const std::string& port)
{
	auto connect = m_network.CreateConnectSocket();
	connect->GetInfo("127.0.0.1", port);
	m_socket = connect->Connect();
	m_socket->SetNonBlocking();
	m_network.AddSocket(m_socket);

	auto maxSize = cCommand::GetMaxRawLength();
	std::string line;

	std::stringstream ss;
	ss << " Valid commands are: " << endl
		<< "       help           - This message" << endl
		<< "       connect {name} - connect as user name" << endl
		<< "       disconnect     - disconnect from the system" << endl
		<< "       create {name}  - Create a new room with {name}" << endl
		<< "       room {name}    - Attach to room {name}" << endl
		<< "       leave          - leave a room" << endl
		<< "       users          - list users" << endl
		<< "       rooms          - list rooms" << endl
		<< "       exit           - Exit the program" << endl;

	m_helpString = ss.str();

	cout << "Welcome to the chat client " << endl << endl
		<< m_helpString;
	m_wedidsomething = true;
	while (!m_exiting)
	{
		if (!m_wedidsomething)
			Sleep(1);				// So we are not hogs
		m_wedidsomething = false;
		// On each output, we need to reprompt the user and echo what they have entered
		// so far
		if (m_reprompt)
		{
			if (m_connectedName.size() > 0)
				cout << "(" << m_connectedName << ") ";
			if (m_connectedRoom.size() > 0)
				cout << "{" << m_connectedRoom << "} ";
			cout << "Command: " << line;
			m_reprompt = false;
		}
		if (_kbhit())
		{
			m_wedidsomething = true;
			char ch = _getch();
			switch (ch)
			{
			case ESCAPE:
			{
				m_exiting = true;
				break;
			}
			case BACKSPACE:
			{
				if (line.size() > 0)
				{
					cout << ch << ' ' << ch;
					line.pop_back();
				}
				break;
			}
			case RETURN:
			{
				cout << endl;
				cmdProcess(line.c_str());
				line.clear();
				m_reprompt = true;
				break;
			}
			default:
			{
				line.push_back(ch);
				cout << ch;				// echo it
			}
			}
		}
		inCheckForResponse();
	}

	m_network.RemoveSocket(m_socket);
}

void cChat_Client::cmdProcess(const char* line)
{
	vecTokens tokens;
	std::string delimiters = " \t";
	Tokens(line, tokens, delimiters);
	if (tokens.size() >= 1)
	{
		::ToLower(tokens[0]);
		auto command = tokens[0];
		bool my_validCommand = false;
		if (command == "exit" || command == "end")
		{
			cmdExit(tokens);
			return;
		}
		if (command == "help")
		{
			cout << m_helpString;
			return;
		}

		if (command == "connect")
		{
			cmdConnect(tokens);
			return;
		}

		// If we are not connected.. everything else is invalid
		if (!m_connected && !m_waitingForConnect)
		{
			cout << "Not Connected : " << endl;
			cout << "Please connect first" << endl;
			return;
		} 

		if (command == "users")
		{
			cmdUsers(tokens);
			return;
		}

		if (command == "rooms")
		{
			cmdRooms(tokens);
			return;
		}

		if (command == "disconnect")
		{
			cmdDisconnect(tokens);
			return;
		}

		if (command == "create")
		{
			cmdCreate(tokens);
			return;
		}

		if (command == "room")
		{
			cmdRoom(tokens);
			return;
		}

		if (command == "leave")
		{
			cmdLeave(tokens);
			return;
		}

		if (command == "wait")
		{
			cmdWait(tokens);
			return;
		}
	
		// Treat the whole thing as a message
		cmdMessage(tokens, line);
	}
}

void cChat_Client::cmdConnect(vecTokens tokens)
{
	// Connecting to the server
	if (m_connected)
	{
		cout << "Already Connected as user: " << m_connectedName << endl;
		cout << "Please disconnect first" << endl;
	}
	else if (tokens.size() != 2)
	{
		cout << "Invalid connect command!" << endl;
		cout << "   Valid: connect \"name\" " << endl;
	}
	else
	{
		m_connectedName = tokens[1];
		::ToLower(m_connectedName);
		cCommand::tUID uid = 0;
		cCommand::tRoom room = 0;
		unsigned short size = (unsigned short)m_connectedName.size();
		cCommand command(cCommand::eConnect, uid, room, m_connectedName.c_str(), size);
		if (gDebug)
		{
			cout << "Sending connect command: " << endl;
			cout << command;
		}
		m_socket->Send(command.GetRawMessage(), command.GetRawLength());
		m_waitingForConnect = true;
	}
}

void cChat_Client::cmdDisconnect(vecTokens tokens)
{
	// Disconnect from  the server
	cCommand::tUID uid = m_UID;
	cCommand::tRoom room = 0;
	cCommand command(cCommand::eDisconnect, uid, room, "", 0);
	if (gDebug)
	{
		cout << "Sending disconnect command: " << endl;
		cout << command;
	}
	m_socket->Send(command.GetRawMessage(), command.GetRawLength());
	RemoveUID(m_UID);
	m_UID = 0;
	m_connectedName = "";
	m_connected = false;
}

void cChat_Client::cmdCreate(vecTokens tokens)
{
	if (tokens.size() != 2)
	{
		cout << "Invalid create command!" << endl;
		cout << "   Valid: create \"name\" " << endl;
	}
	else
	{
		std::string roomname = tokens[1];
		::ToLower(roomname);
		auto roomID = GetRoom(roomname);
		if (roomID != cCommand::tRoom_Unknown)
		{
			cout << "Room already exists - "
				<< " ID: " << roomID
				<< " Name: " << roomname << endl;
		}
		else
		{
			cCommand::tUID uid = 0;
			cCommand::tRoom room = 0;
			unsigned short size = (unsigned short)roomname.size();
			cCommand command(cCommand::eRoomCreate, uid, room, roomname.c_str(), size);
			if (gDebug)
			{
				cout << "Sending Room Create command: " << endl;
				cout << command;
			}
			m_socket->Send(command.GetRawMessage(), command.GetRawLength());
		}
	}
}

void cChat_Client::cmdRoom(vecTokens tokens)
{
	//  "room"			-- Switch to talking on that chat room
	if (tokens.size() != 2)
	{
		cout << "Invalid room command!" << endl;
		cout << "   Valid: room \"roomname\" " << endl;
	}
	else
	{
		std::string roomname = tokens[1];
		::ToLower(roomname);
		cCommand::tRoom room = GetRoom(roomname);
		if (room == cCommand::tRoom_Unknown)
		{
			cout << "Invalid room name: " << roomname << endl;
		}
		else
		{
			cCommand::tUID uid = 0;
			unsigned short size = (unsigned short)roomname.size();
			cCommand command(cCommand::eRoomConnect, m_UID, room, roomname.c_str(), size);
			if (gDebug)
			{
				cout << "Sending Room Create command: " << endl;
				cout << command;
			}
			m_socket->Send(command.GetRawMessage(), command.GetRawLength());
		}
	}
}

void cChat_Client::cmdLeave(vecTokens tokens)
{
	if (m_Room == cCommand::tRoom_Unknown)
	{
		cout << "Not connected to room " << endl;
		return;
	}

	cCommand command(cCommand::eRoomLeave, m_UID, m_Room, "", 0);
	if (gDebug)
	{
		cout << "Leaving Room: " << endl;
		cout << command;
	}
	m_socket->Send(command.GetRawMessage(), command.GetRawLength());
	m_Room = cCommand::tRoom_Unknown;
	m_connectedRoom = "";
}

void cChat_Client::cmdWait(vecTokens tokens)
{
	//  "wait"			-- Switch to talking on that chat room
	//  "wait" - waits on second
	//  "wait n" - waits n seconds
	//  "wait nn mm" - waits random between nn and mm
	int first = (tokens.size() >= 2) ? std::stoi(tokens[1]) : 1;
	int second = (tokens.size() >= 3) ? std::stoi(tokens[2]) : 0;
	int ms = 1000 * (second > first) ? (rand() % second + first) : first;
	Sleep(ms);
}

void cChat_Client::cmdMessage(vecTokens tokens, const char* line)
{
	//  "message"			-- Switch to talking on that chat room
	//   If we are in a room, just send the text to that room
	if (m_Room == cCommand::tRoom_Unknown)
	{
		cout << "Not connected to a room" << endl;
		cout << " Enter room \"room name\"" << endl;
	}
	else
	{
		unsigned short totalsize = (unsigned short)std::string(line).size();
		unsigned short offset = 0;
		while (totalsize > 0)
		{
			unsigned short size = totalsize;
			if (size > cCommand::GetMaxMessageLength())
			{
				size = cCommand::GetMaxMessageLength();
			}
			totalsize -= size;
			cCommand command(cCommand::eMessage, m_UID, m_Room, line+offset, size);
			if (gDebug)
			{
				cout << "Sending a message to room: " << m_Room << endl;
				cout << command;
			}
			m_socket->Send(command.GetRawMessage(), command.GetRawLength());
			offset += size;
		}
	}
}

void cChat_Client::cmdRooms(vecTokens tokens)
{
	auto rooms = m_Rooms.GetMapRooms();
	if (rooms.size() == 0)
		cout << "No rooms available" << endl;
	else
	{
		for (auto room : rooms)
		{
			cout << "Room ID: " << room.first
				 << " Name: " << room.second->GetName() << endl;
		}
	}
}

void cChat_Client::cmdUsers(vecTokens tokens)
{
	for (auto user : m_All_UIDs)
	{
		cout << "User ID: " << user.first
			<< " Name: " << user.second << endl;
	}
}

void cChat_Client::cmdExit(vecTokens tokens)
{
	m_exiting = true;
}

void cChat_Client::inCheckForResponse()
{
	auto maxSize = cCommand::GetMaxRawLength();
	char* recvbuf = new char[maxSize];
	bool loop = true;
	while (loop)
	{
		loop = false;
		size_t recv = m_socket->Recieve(recvbuf, maxSize);
		if (recv == 0)
		{
			cout << "Connection closed." << endl;
			m_exiting = true;
		}
		else if (recv == WSAEWOULDBLOCK)
		{
			if (!m_blockingMessage)
			{
				if (gDebug)
				{
					cout << "We are blocking" << endl;
					m_reprompt = true;
				}
			}
			m_blockingMessage = true;
		}
		else if (recv > 0)
		{
			loop = true;
			if (recv > cCommand::GetMaxRawLength())
			{
				// We have an error so disconnect
				m_exiting = true;
			}
			else
			{
				m_reprompt = true;					// Tell outer loop to reprompt the user
				cCommand command(recvbuf, (unsigned short)recv);
				if (gDebug)
				{
					cout << "Bytes received: " << recv << endl;
					cout << command;
				}
				auto cmd = command.GetCommand();
				switch (cmd)
				{
				case cCommand::eConnected:
				{
					inConnected(command);
					break;
				}
				case cCommand::eDisconnected:
				{
					inDisconneted(command);
					break;
				}
				case cCommand::eRoomCreated:
				{
					inRoomCreated(command);
					break;
				}
				case cCommand::eRoomConnected:
				{
					inRoomConnected(command);
					break;
				}
				case cCommand::eRoomLeft:
				{
					inRoomLeft(command);
					break;
				}
				case cCommand::eMessage:
				{
					inMessage(command);
					break;
				}
				default:
				{
					// Something unexpected was recieved
					cout << endl << "Unexpected command recieved: " << cmd;
					cout << command;
					break;
				}
				}
			}
			m_wedidsomething = true;
		}
	}
	delete[] recvbuf;
}

void cChat_Client::inConnected(cCommand& command)
{
	auto uid = command.GetUID();
	auto room = command.GetRoom();
	auto message = command.GetTheMessage();

	AddUID(uid, message);

	if (m_waitingForConnect && message == m_connectedName)
	{
		m_waitingForConnect = false;
		m_UID = uid;
		m_connected = true;
	}
	m_reprompt = false;			// No message so don't reprompt
}

void cChat_Client::inDisconneted(cCommand& command)
{
	auto uid = command.GetUID();
	auto room = command.GetRoom();
	auto message = command.GetTheMessage();
	// Someone disconneted from the system
	cout << endl << "User Disconnected: "
		<< "UID: " << uid <<
		" Name: " << GetUIDName(uid) << endl;
	RemoveUID(uid);
	if (uid == m_UID)
	{
		// It's me who disconnected
		m_UID = 0;
		m_connectedName = "";
		m_connected = false;
	}
}

void cChat_Client::inRoomCreated(cCommand& command)
{
	auto uid = command.GetUID();
	auto room = command.GetRoom();
	auto message = command.GetTheMessage();
	// A new room was created
	AddRoom(room, message);
	cout << endl << "A new room was created: "
		<< "ID: " << room
		<< " Name: " << message << endl;
}

void cChat_Client::inRoomConnected(cCommand& command)
{
	auto uid = command.GetUID();
	auto room = command.GetRoom();
	auto message = command.GetTheMessage();

	// New room created
	cout << endl << "A user connected to a room: "
		<< "UID: " << uid
		<< "Room: " << room
		<< " Name: " << message
		<< " User: " << GetUIDName(uid) << endl;

	AddRoom(room, message);
	if (uid == m_UID)
	{
		// Created by me.. so note it
		m_Room = room;
		m_connectedRoom = message;
	}
}

void cChat_Client::inRoomLeft(cCommand& command)
{
	auto uid = command.GetUID();
	auto room = command.GetRoom();
	auto message = command.GetTheMessage();

	if (uid != m_UID)
	{
		// New room created
		cout << endl << "A user left room: "
			<< "UID: " << uid
			<< "Room: " << room
			<< " Name: " << message
			<< " User: " << GetUIDName(uid) << endl;

	} else
	{
		// Created by me.. so note it
		m_Room = room;
		m_connectedRoom = message;
	}
}

void cChat_Client::inMessage(cCommand& command)
{
	auto uid = command.GetUID();
	auto room = command.GetRoom();
	auto message = command.GetTheMessage();

	if (uid != m_UID)
	{
		cout << endl << " - (" << GetUIDName(uid) << ") "
			<< "{" << GetRoomName(room) << "} "
			<< message << endl;
	}
	//cout << endl << "A message was recieved: "
	//	<< "UID: " << uid
	//	<< "Room: " << room
	//	<< " Name: " << message
	//	<< " User: " << GetUIDName(uid) << endl;								// A message was sent
	//if (uid == m_UID)
	//	cout << "This was sent by myself" << endl;
}
