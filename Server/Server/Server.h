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

using namespace std;


// Different packets
enum Packet {
	P_ChatMessage
};


class Server {

public:

	// constructor
	Server(int PORT, bool BroadcastPublically = false); 
	bool ListenForNewConnection();

private:
	
	bool ProcessPacket(int id, Packet packettype);
	static void ClientHandlerThread(int id);

	// Sending functions
	bool sendall(int id, char* data, int totalbytes);
	bool SendInt(int id, int _int);
	bool SendPacketType(int id, Packet _packettype);
	bool SendString(int id, string &_string);

	// Getting functions
	bool recvall(int id, char* data, int totalbytes);
	bool GetInt(int id, int &_int);
	bool GetPacketType(int id, Packet &_packettype);
	bool GetString(int id, string &_string);


private:
	 
	SOCKET Connections[MAX_CONNECTIONS]; // Array of all clients that can connect to server
	int TotalConnections = 0;			 // Total number of clients

	SOCKADDR_IN addr; 			// Address that we shall bind our listening socket to
	int addrlen = sizeof(addr);	// Length of the address

	SOCKET sListen; // Socket used to listen for new connections

};

// Allows for static ClientHandlerThread function to access instance members
static Server* serverptr;

#endif 
