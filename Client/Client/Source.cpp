/** Client

Communication pipeline:
	1. Send our packet type
	2. Initially send integer that will indicate size (in bytes) of incomming packet
	3. Send our packet
*/

// Includes
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <string>
#include <WinSock2.h>

#define HOST_IP "127.0.0.1"
#define HOST_PORT 1111

using namespace std;

SOCKET Connection;

// Different packets
enum Packet {
	P_ChatMessage,
	P_Test
};

bool ProcessPacket(Packet packettype) {

	switch (packettype) {

	case P_ChatMessage:
	{

		int bufferlen; // length of recived chat message 

		recv(Connection, (char*)&bufferlen, sizeof(int), NULL);  // recieve buffer length
		char* buffer = new char[bufferlen + 1]; 				 // create a new recieve buffer
		buffer[bufferlen] = '\0';								 // set last character to null terminator
		recv(Connection, buffer, bufferlen, NULL); 		         // accept message

		cout << buffer << endl; // print out message
		delete[] buffer; 		// dealocate buffer
		break;
	}

	case P_Test:
		cout << "Recieved test packet!" << endl; 
		break;

	// handle unrecognized packets
	default:
		cout << "Unrecognized packet: " << packettype << endl;
		break;

	}


	return true;
}

void ClientThread() {
	

	Packet packettype;
	while (true) {
		
		recv(Connection, (char*)&packettype, sizeof(Packet), NULL); // receive packet type
	
		if (!ProcessPacket(packettype)) // if packet is not properly processed, break from client-handler loop
			break; 
		
	}

	closesocket(Connection); // Close my socket when done
}

int main(int argc, char** argv) {

	cout << "client running...\n";


	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);

	// If error occurs during WSAStartup 
	if (WSAStartup(DllVersion, &wsaData) != 0) {

		MessageBoxA(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		
		// Exit program
		exit(1);
	}

	// Address that we shall bind our listening socket to
	SOCKADDR_IN addr;

	// Length of the address 
	int addrlen = sizeof(addr);

	addr.sin_addr.s_addr = inet_addr(HOST_IP); 	// Connect locally
	addr.sin_port = htons(HOST_PORT); // Port
	addr.sin_family = AF_INET; // IPv4 Socket


	// set connection socket
	Connection = socket(AF_INET, SOCK_STREAM, NULL);

	// Attempt to establish a connection
	if (connect(Connection, (SOCKADDR*) &addr, addrlen) != 0){
		// If error occurs...

		MessageBoxA(NULL, "Failed to Connect..", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	cout << "Connected!\n";

	/*
	// declare array to recieve message from server
	char message[256];

	// Receive message from server
	recv(Connection, message, sizeof(message), NULL);
	std::cout << "Server said: " << message << '\n';
	*/

	// Create client thread
	CreateThread(NULL, NULL,
		(LPTHREAD_START_ROUTINE)ClientThread,
		NULL, NULL, NULL
	);


	// -- Outgoing message handling -- //

	string buffer; // output buffer
	while (true) {

		getline(cin, buffer);			// get input from user
		int bufferlen = buffer.size();	// get size of input
		
		Packet packettype = P_ChatMessage;							 // create packet type to chat message
		send(Connection, (char *)&packettype, sizeof(Packet), NULL); // send packet type
		send(Connection, (char*)&bufferlen, sizeof(int), NULL);		 // send message length
		send(Connection, buffer.c_str(), bufferlen, NULL);			 // send message

		Sleep(10); // Sleep
	}


	system("pause");
	return 0;
}