#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>


#include "../Sockets/cNetwork.h"
#include "../Sockets/cListenSocket.h"
#include "../Sockets/cSocketException.h"

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
		do
		{
			char recBuffer[1024];
			size_t recBufferLength = sizeof(recBuffer);
			recieved = sock->Recieve(recBuffer, recBufferLength);
			sock->Send(recBuffer, recieved);
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