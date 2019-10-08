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
#include "../Chat/cChat_Client.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Sockets.lib")
#pragma comment (lib, "Chat.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5150"

bool gDebug = false;		// turn off global debug

using namespace std;

int main(int argc, char** argv)
{
	cNetwork network;
	try {
		network.Initialize();
		cChat_Client chat(network);
		chat.Process(DEFAULT_PORT);
		network.Cleanup();
	}
	catch (std::exception& ex)
	{
		std::cout << "exception caught" << ex.what() << std::endl;
		network.Cleanup();			// This will shut down everything
	}
}
