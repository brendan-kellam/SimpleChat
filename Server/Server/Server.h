#ifndef SERVER_H
#define SERVER_H

#pragma once
#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define LOCAL_IP "127.0.0.1"
#define MAX_CONNECTIONS 100

#include <iostream>
#include <string>
#include <WinSock2.h>
#include "FileTransferData.h"
#include "PacketType.h"

using namespace std;

struct Connection
{
	// Socket to client
	SOCKET socket;

	// FileTransferData for given client
	FileTransferData file;
};


class Server {

public:

	// constructor
	Server(int PORT, bool BroadcastPublically = false); 
	bool ListenForNewConnection();

private:
	
	bool ProcessPacket(int id, PacketType packettype);
	bool SendFileByteBuffer(int id);

	static void ClientHandlerThread(int id);

	// Sending functions
	bool sendall(int id, char* data, int totalbytes);
	bool SendInt32_t(int id, int32_t _int32_t);
	bool SendPacketType(int id, PacketType _packettype);
	bool SendString(int id, string &_string);

	// Getting functions
	bool recvall(int id, char* data, int totalbytes);
	bool GetInt32_t(int id, int32_t &_int32_t);
	bool GetPacketType(int id, PacketType &_packettype);
	bool GetString(int id, string &_string);


private:
	 
	Connection connections[MAX_CONNECTIONS]; // Array of all clients that can connect to server
	int TotalConnections = 0;			 // Total number of clients

	SOCKADDR_IN addr; 			// Address that we shall bind our listening socket to
	int addrlen = sizeof(addr);	// Length of the address

	SOCKET sListen; // Socket used to listen for new connections

};

// Allows for static ClientHandlerThread function to access instance members
static Server* serverptr;

#endif 
