#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <stdlib.h>
#include <string>

using namespace std;

int main(int argc, char** argv)
{
	std::string path = "cmd ../x64/Debug/";
	std::string client = path + "client.exe";
	std::string server = path + "server.exe";
	system("cmd&");
	system(server.c_str());

	Sleep(2000);
	system(client.c_str());
}