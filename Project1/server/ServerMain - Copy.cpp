#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>


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

int main(int argc, char** argv)
{
	cNetwork network;
	try {
		network.Initialize();
		auto listen = network.CreateListenSocket(10);
		listen.GetInfo(DEFAULT_PORT);
		listen.Create();
		listen.Bind();
		listen.Listen();
		cSocket* sock = listen.Accept();
		network.AddSocket(sock);

		std::cout << "Waiting to receive data from the client..." << endl;
		size_t recieved;
		cChat chat;
		size_t recBufferLength = cCommand::GetMaxRawLength();
		char* recBuffer = new char[recBufferLength];

		do
		{
			recieved = sock->Recieve(recBuffer, recBufferLength);
			cCommand command(recBuffer, (unsigned short)recieved);
			cout << command;
			auto cmd = command.GetCommand();
			auto uid = command.GetUID();
			auto room = command.GetRoom();
			auto message = command.GetTheMessage();
			auto size = command.GetMessageLength();
			switch (cmd)
			{
			case cCommand::eConnect:
			{
				room = 0;
				uid = chat.GetNextFreeUID();
				// Broadcast Connect to everyone
				cout << "Adding UID: " << uid << " - " << message << endl;
				chat.AddUID(uid, message);		// Add the new user
				cCommand response(cCommand::eConnected, uid, room, message, size);
				sock->Send(response.GetRawMessage(), response.GetRawLength());
				break;
			}
			case cCommand::eDisconnect:
			{
				room = 0;
				cout << "RemoveUID: " << uid << " - " << message << endl;
				chat.RemoveUID(uid, message);		// Remove the user
				// Broadcast Disconnect to everyone
				cCommand response(cCommand::eDisconnected, uid, room, message, size);
				cout << "Responding to disconnect :" << endl << response;
				sock->Send(response.GetRawMessage(), response.GetRawLength());
				break;
			}
			case cCommand::eRoomCreate:
			{
				break;
			}
			case cCommand::eRoomConnect:
			{
				break;
			}
			case cCommand::eMessage:
			{
				break;
			}
			default:
			{
			}
			}
		} while (recieved != 0);
		network.RemoveSocket(sock);
		network.Cleanup();
	}
	catch (std::exception& ex)
	{
		std::cout << "exception caught" << ex.what() << std::endl;
		network.Cleanup();			// This will shut down everything
	}
}