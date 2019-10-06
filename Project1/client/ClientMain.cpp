#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <istream>
#include <vector>
#include <algorithm>
#include <sstream>

#include "../Sockets/cNetwork.h"
#include "../Sockets/cListenSocket.h"
#include "../Sockets/cSocketException.h"
#include "../Chat/cCommand.h"
#include "../Chat/cChat.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Sockets.lib")
#pragma comment (lib, "Chat.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5150"

using namespace std;

void Tokens(const std::string& in, std::vector<std::string>& out, const string& separators)
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

int main(int argc, char** argv)
{
	cNetwork network;
	try {
		network.Initialize();
		auto connect = network.CreateConnectSocket();
		connect.GetInfo("127.0.0.1", DEFAULT_PORT);
		cSocket* sock = connect.Connect();
		network.AddSocket(sock);

		auto maxSize = cCommand::GetMaxRawLength();
		char* line = new char[maxSize];
		char* recvbuf = new char[maxSize];

		cChat chat;
		bool my_exiting = false;
		bool my_connected = false;
		bool my_waitingForConnect = false;
		bool my_waitingForResponse = false;
		cCommand::tUID my_UID = cCommand::tUID_Unknown;
		std::string my_connectedName;
		cCommand::tRoom my_Room = cCommand::tRoom_Unknown;
		std::string my_connectedRoom;
		while (!my_exiting)
		{
			if (my_connectedName.size() > 0)
				cout << "(" << my_connectedName << ") ";
			if (my_connectedRoom.size() > 0)
				cout << "{" << my_connectedRoom << "} ";
			cout << "Command: ";
			std::cin.getline(line, sizeof(line));
			vector <string> tokens;
			std::string delimiters = " \t";
			Tokens(line, tokens, delimiters);
			if (tokens.size() >= 1)
			{
				::ToLower(tokens[0]);
				auto command = tokens[0];
				bool my_validCommand = false;
				if (command == "exit" || command == "end")
				{
					my_exiting = true;
					my_validCommand = true;
				} //if (command == "exit")
				
				if (command == "connect")
				{
					// Connecting to the server
					if (my_connected)
					{
						cout << "Already Connected as user: " << my_connectedName << endl;
						cout << "Please disconnect first" << endl;
					}
					else if (tokens.size() != 2)
					{
						cout << "Invalid connect command!" << endl;
						cout << "   Valid: connect \"name\" " << endl;
					}
					else
					{
						my_connectedName = tokens[1];
						::ToLower(my_connectedName);
						cCommand::tUID uid = 0;
						cCommand::tRoom room = 0;
						unsigned short size = (unsigned short)my_connectedName.size();
						cCommand command(cCommand::eConnect, uid, room, my_connectedName.c_str(), size);
						cout << "Sending connect command: " << endl;
						cout << command;
						sock->Send(command.GetRawMessage(), command.GetRawLength());
						my_waitingForResponse = my_waitingForConnect = true;
					}
					my_validCommand = true;
				} // if (command == "connect")

				if (!my_connected)
				{
					cout << "Not Connected : " << endl;
					cout << "Please connect first" << endl;
					my_validCommand = true;
				} // if (!connected)

				if (command == "disconnect")
				{
						// Disconnect from  the server
						cCommand::tUID uid = my_UID;
						cCommand::tRoom room = 0;
						cCommand command(cCommand::eDisconnect, uid, room, "", 0);
						cout << "Sending disconnect command: " << endl;
						cout << command;

						sock->Send(command.GetRawMessage(), command.GetRawLength());
						my_waitingForResponse = true;
						my_validCommand = true;
				} // if (command == "disconnect")
				
				if (command == "create")
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
						cCommand::tUID uid = 0;
						cCommand::tRoom room = 0;
						unsigned short size = (unsigned short)roomname.size();
						cCommand command(cCommand::eRoomCreate, uid, room, roomname.c_str(), size);
						cout << "Sending Room Create command: " << endl;
						cout << command;
						sock->Send(command.GetRawMessage(), command.GetRawLength());
						my_waitingForResponse = true;
					}
					my_validCommand = true;
				} // if (command == "create")

				if (command == "room")
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
						cCommand::tRoom room = chat.GetRoom(roomname);
						if (room == cCommand::tRoom_Unknown)
						{
							cout << "Invalid room name: " << roomname << endl;
						}
						else
						{
							cout << "Connecting to Room command: " << roomname << endl;
							my_connectedRoom = roomname;
							my_Room = room;
						}
					}
					my_validCommand = true;
				}
				if (command == "leave")
				{
					// Leave 				-- Leave a chat room
					my_connectedRoom = "";
					my_Room = cCommand::tRoom_Unknown;
					my_validCommand = true;
				} // else if (command == "leave")
				if (command == "wait")
				{
					//  "wait"			-- Switch to talking on that chat room
					//  "wait" - waits on second
					//  "wait n" - waits n seconds
					//  "wait nn mm" - waits random between nn and mm
					int first = (tokens.size() >= 2) ? std::stoi(tokens[1]) : 1;
					int second = (tokens.size() >= 3) ? std::stoi(tokens[2]) : 0;
					int ms = 1000 * (second > first) ? (rand() % second + first) : first;
					Sleep(ms);
					my_validCommand = true;
				} // if (command == "wait")
				if (command == "message")
				{

					//  "message"			-- Switch to talking on that chat room
					//   If we are in a room, just send the text to that room
					if (my_Room == cCommand::tRoom_Unknown)
					{
						cout << "Not connected to a room" << endl;
						cout << " Enter room \"room name\"" << endl;
					}
					else
					{
						unsigned short size = (unsigned short)std::string(line).size();
						cCommand command(cCommand::eMessage, my_UID, my_Room, line, size);
						cout << "Sending a message to room: " << my_Room << endl;
						cout << command;
						sock->Send(command.GetRawMessage(), command.GetRawLength());
						my_waitingForResponse = true;	// it will be returned to us
					}

					my_validCommand = true;
				} // if (command == "message")

				if (!my_validCommand)
				{
					cout << "Invalid connect command!" << endl;
					cout << "   Valid: connect \"name\" " << endl;
				}
				// We are not multi threaded yet
				if (my_waitingForResponse)
				{
					// Receive a message from the server before quitting
					cout << "Waiting to receive data from the server..." << endl;
					size_t recv = sock->Recieve(recvbuf, sizeof(recvbuf));
					if (recv > 0)
					{
						cout << "Bytes received: " << recv << endl;
						cCommand command(recvbuf, (unsigned short)recv);
						cout << command;
						// Pull everything out of the recieved buffer
						auto cmd = command.GetCommand();
						auto uid = command.GetUID();
						auto message = command.GetTheMessage();
						auto length = command.GetMessageLength();
						auto room = command.GetRoom();
						switch (cmd)
						{
							case cCommand::eConnected:
							{
								if (my_waitingForConnect && message == my_connectedName)
								{
									my_waitingForConnect = false;
									chat.AddUID(uid, message);
									my_UID = uid;
									my_connected = true;
								}
								// Someone conected to the system
								break;
							}
							case cCommand::eDisconnected:
							{
								// Someone disconneted from the system
								cout << "User Disconnected: "
									<< "UID: " << uid << 
									" Name: " << message << endl;
								chat.RemoveUID(uid, message);
								if (uid == my_UID)
								{
									// It's me who disconnected
									my_UID = 0;
									my_connectedName = "";
									my_connected = false;
								}
								break;
							} //cCommand::eDisconnected:
							case cCommand::eRoomCreated:
							{
								// A new room was created
								chat.AddRoom(room, message);
								cout << "A new room was created: "
									 << "ID: " << room 
									 << " Name: " << message << endl;
								break;
							} //case cCommand::eRoomCreated:
							case cCommand::eRoomConnected:
							{
								// Someone connected to a room
								cout << "A user connected to a room: "
									 << "UID: " << uid
									 << "Room: "  << room
									 << " Name: " << message 
									 << " User: " << chat.GetUIDName(uid) << endl;
								chat.AddRoom(uid, message);
								break;
							} //case cCommand::eRoomConnected:
							case cCommand::eMessage:
							{
								cout << "A message was recieved: "
									<< "UID: " << uid
									<< "Room: " << room
									<< " Name: " << message
									<< " User: " << chat.GetUIDName(uid) << endl;								// A message was sent
								if (uid == my_UID)
									cout << "This was sent by myself" << endl;
								break;
							} //case cCommand::eMessage:
							default:
							{
								// Something unexpected was recieved
									cout << "Unexpected command recieved: "
									<< "Command: " << cmd
									<< "ID: " << uid
									<< "Room: " << room
									<< " Name: " << message
									<< " User: " << chat.GetUIDName(uid) << endl;								// A message was sent
								cout << command;
							} //default:
						} //switch (cmd)
						my_waitingForResponse = false;
					} //if (my_waitingForResponse)
					else if (recv == 0)
					{
						cout << "Connection closed." << endl;
					} //else if (recv == 0)

				} // if (waitingForResponse)
			} // if (tokens.size() >= 1)
		} // (!exiting)

		delete[] recvbuf;
		delete[] line;

		network.RemoveSocket(sock);
		network.Cleanup();
	}
	catch (std::exception& ex)
	{
		std::cout << "exception caught" << ex.what() << std::endl;
		network.Cleanup();			// This will shut down everything
	}
}
