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
#include "FileTransferData.h"

using namespace std;


// Packet type enumeration
enum Packet {
	P_ChatMessage,					// Send simple chat message
	P_FileTransferRequestFile,	    // [C->S] Request a file to transfer
	P_FileTransfer_EndOfFile,       // [S->C] Sent for when file transfer is completed
	P_FileTransferByteBuffer,       // [S->C] Sent before sending byte buffer for file transfer
	P_FileTransferRequestNextBuffer // [C->S] Sent to request the next buffer for file
};



class Client {

public:

	Client(string IP, int PORT); // private constructor

	bool Connect();
	bool CloseConnection();

	bool SendString(string &_string, bool IncludePacketType = true);
	bool RequestFile(string FileName);

private:
	bool ProcessPacket(Packet _packettype); // Process a new packettype
	static void ClientThread();				// Thread for handling incoming packets {~static~} [seperate thread]

	// Sending functions
	bool sendall(char* data, int totalbytes);
	bool SendInt32_t(int32_t _int32_t);
	bool SendPacketType(Packet _packettype);

	// Getting functions
	bool recvall(char* data, int totalbytes); 
	bool GetInt32_t(int32_t &_int32_t);					  
	bool GetPacketType(Packet &_packettype);  
	bool GetString(string &_string);		 




private: 
	FileTransferData file;			// Object that containes information about our file
	SOCKET Connection;				// Client's connection to server
	SOCKADDR_IN addr;				// Address to be binded to our Connection sockets
	int sizeofaddr = sizeof(addr);  // Needed for connect function

};

// Only one client
static Client* clientptr;
