Course: Info6016 - Networking
Name: Henry Van Horik
Student: 0966956

Targets available: Debug/Release/x64/x86
Build directory example: "./Project1\x64\Release"
Application names:	server.exe client.exe
[Build All] project will build everything you need

**** NOTE - The server does a load of asserts checking for invalid incoming messages which will halt
the program if you use the debug build.

**** NOTE - The maximum buffer length has been set to 512 (beyond that your socket is shut down)
**** NOTE - My protocol is nonstandard as noted below.

 Protocol (All messages are identical and unused fields are ignored):
                                                Header 
    uLong          char       uShort   uShort      uShort        char[]  
[packet_length] [message_id] [User ID][Room ID][Message Length][Message]

Valid commands are:
		eNop = 0,
		eConnect,				- User
		eConnected,				- Server
		eDisconnect,			- User
		eDisconnected,			- Server
		eRoomCreate,			- User
		eRoomCreated,			- Server
		eRoomConnect,			- User
		eRoomConnected,			- Server
		eRoomLeave,				- User
		eRoomLeft,			  	- Server
		eMessage,				- User/Server

To run:
   Launch server.exe
   Launch: client.exe (multiple copies)
Commands:
       help           - This message
       connect {name} - connect as user name
       disconnect     - disconnect from the system
       create {name}  - Create a new room with {name}
       room {name}    - Attach to room {name}
       leave          - leave a room
       users          - list users
       rooms          - list rooms
       exit           - Exit the program


Example:
	Command: connect henry2     		// connecting
	(henry2) Command:
	A new room was created: ID: 2 Name: foo
	(henry2) Command: room foo 	 	 // Entering a room
	(henry2) Command:
	A user connected to a room: UID: 4Room: 2 Name: foo User: henry2
	(henry2) {foo} Command: hello Henry
	(henry2) {foo} Command:
	 - (henry) {foo} Hello henry2			 // A message from henry
	(henry2) {foo} Command:
	A user left room: UID: 3Room: 2 Name:  User: henry
	(henry2) {foo} Command:
	A new room was created: ID: 3 Name: foo2
	(henry2) {foo} Command:
	User Disconnected: UID: 3 Name: henry
	(henry2) {foo} Command:
	(henry) Command: rooms
	Room ID: 2 Name: foo
	Room ID: 3 Name: foo2
	(henry) Command: users
	User ID: 4 Name: henry2
	User ID: 5 Name: henry
	(henry) Command:

The protocol is async. 
You send a connection request , (eConnect) and the server responds with a (eConnected)

eConnected, eDisconnected, eRoomCreated are broadcast to all users
eRoomConnected, eRoomLeave and eMessage - only broadcast to members of the room

When you eConnect to the system, all users and rooms are broadcast to you for parsing.
I had made an early design decision to just pass UID and Room IDs rather than strings containing 
the user and room names. I figured this would be less wasteful. Each user keeps an active list 
of UID/Name and Room/Name maps on their system. That’s why the server will broadcast all users 
and names on your eConnect. 

NOTE - **Probably would design it differently in the future but I was stuck.

Both server and client are non-blocking.

There are two library projects
Socket.vcproj - This has a socket wrapper
Chat.vcproj - This implements the chat protocol
   cCommand.h/cpp - This implements the packing/unpacking of the commands to be sent
   cChat_Server.h/cpp - This handles the server protocol
   cChat_Client.h/cpp - This handles the client protocol and keyboard commands.

Note - since I serialized and deserialized in place directly into the buffer there was no 
reason to keep buffer position. cCommand.h/cpp just wraps the raw Big endian buffer and 
packs/unpacks as required by the user calls. Overflow is tested when adding and reading the 
actual message. If a message exceeds the buffer size it is broken into two messages in the 
Client.

