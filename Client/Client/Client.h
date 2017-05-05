/** 
Client: Provides connection to a given server.

Communication pipeline:
1. Send our packet type
2. Initially send integer that will indicate size (in bytes) of incomming packet
3. Send our packet
*/

#pragma once
#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <WinSock2.h>

using namespace std;


// Packet type enumeration
enum Packet {
	P_ChatMessage
};


class Client {

public:

	Client(string IP, int PORT); // private constructor

	bool Connect();
	bool CloseConnection();

	bool SendString(string &_string);

private:
	bool ProcessPacket(Packet _packettype);
	static void ClientThread();

	// Sending functions
	bool SendInt(int _int);
	bool SendPacketType(Packet _packettype);

	// Getting functions
	bool GetInt(int &_int);
	bool GetPacketType(Packet &_packettype);
	bool GetString(string &_string);

private: 
	static Client* instance;

	SOCKET Connection;				// Client's connection to server
	SOCKADDR_IN addr;				// Address to be binded to our Connection sockets
	int sizeofaddr = sizeof(addr);  // Needed for connect function

};

// Only one client
static Client* clientptr;
