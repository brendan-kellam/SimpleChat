/** 
Client: Provides connection to a given server.

Communication pipeline:
1. Send our PacketType
2. Initially send integer that will indicate size (in bytes) of incomming packet
3. Send our packet
*/

#pragma once
#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <WinSock2.h>
#include "FileTransferData.h"
#include "PacketType.h"

using namespace std;



class Client {

public:

	Client(string IP, int PORT); // private constructor

	bool Connect();
	bool CloseConnection();

	bool SendString(string &_string, bool IncludePacketType = true);
	bool RequestFile(string FileName);

private:
	bool ProcessPacket(PacketType _packettype); // Process a new packettype
	static void ClientThread();				// Thread for handling incoming packets {~static~} [seperate thread]

	// Sending functions
	bool sendall(char* data, int totalbytes);
	bool SendInt32_t(int32_t _int32_t);
	bool SendPacketType(PacketType _packettype);

	// Getting functions
	bool recvall(char* data, int totalbytes); 
	bool GetInt32_t(int32_t &_int32_t);					  
	bool GetPacketType(PacketType& _packettype);  
	bool GetString(string &_string);		 




private: 
	FileTransferData file;			// Object that containes information about our file
	SOCKET Connection;				// Client's connection to server
	SOCKADDR_IN addr;				// Address to be binded to our Connection sockets
	int sizeofaddr = sizeof(addr);  // Needed for connect function

};

// Only one client
static Client* clientptr;
