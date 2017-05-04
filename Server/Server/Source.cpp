/**
Server
***/

// includes  

// Allows to access functions
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <WinSock2.h>

#define HOST_IP "127.0.0.1"
#define HOST_PORT 1111
#define MAX_CONNECTIONS 100

using namespace std;

// Array of connections
SOCKET Connections[MAX_CONNECTIONS];
int ConnectionCounter = 0;

// Different packets
enum Packet {
	P_ChatMessage,
	P_Test
};

bool ProcessPacket(int id, Packet packettype) {

	switch (packettype) {
	case P_ChatMessage:
	{

		int bufferlen; // length of recived message
		recv(Connections[id], (char*)&bufferlen, sizeof(int), NULL); // Get buffer length 

		char* buffer = new char[bufferlen];             // allocate memory for recieved message
		recv(Connections[id], buffer, bufferlen, NULL); // recieve message from originating client

		// loop all connected clients
		for (int i = 0; i < ConnectionCounter; i++) {

			if (i == id) continue; 	// Skip originating client

			send(Connections[i], (char*)&packettype, sizeof(Packet), NULL); // send chat message packet type
			send(Connections[i], (char*)&bufferlen, sizeof(int), NULL);     // send length of chat message
			send(Connections[i], buffer, bufferlen, NULL);					// send chat message
		} 

		delete[] buffer; // deallocate memory
		break; 
	}

	default:
		cout << "Unrecognized packet: " << packettype << endl;
		break;
	
	}


	return true;
}

void ClientHandlerThread(int id) {
	
	Packet packettype;
	while (true) {

		recv(Connections[id], (char*)&packettype, sizeof(Packet), NULL); // Receive packet type from client

		if (!ProcessPacket(id, packettype)) 
			break;
	}

	closesocket(Connections[id]); //when done with a client, close the socket

}

int main(int argc, char** argv) {

	cout << "server starting...\n";

	// Winsock Startup
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


	// Create socket to listen for new connections
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	
	// Bind the address to the socket
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));

	// Places the socket in a state such that it is waiting for a incoming connection
	listen(sListen, SOMAXCONN); // "SOMAXCONN": Socket outstanding maxiumum connectiosn
								// (The total amount of people that can try and connect at once)


	// Socket to hold the client's connection
	SOCKET newConnection;

	// Loop all possible connections connecting
	for (int i = 0; i < MAX_CONNECTIONS; i++) {

		// Accept a new client connection (accept is most likely running on another thread)
		newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen);

		// Check if accepting client connection failed 
		if (newConnection == 0) {
			cout << "Failed to accept client connection...\n";
		}
		
		// If the client connection was accepted
		else {
			cout << "Client connected!\n";
			
			int id = i;
			Connections[id] = newConnection; // add connection to array 
			ConnectionCounter++;			// increment connections counter
			CreateThread(					// create thread for new client			   
				NULL, NULL,	
				(LPTHREAD_START_ROUTINE)ClientHandlerThread,
				(LPVOID)(id), // parameter
				NULL, NULL
			);


			Packet packettype = P_ChatMessage;		   // set packet type
			string message = "Welcome to the server!"; // create welcome message
			int messagelen = message.size();

			send(newConnection, (char*)&packettype, sizeof(Packet), NULL); // send chat message packet type
			send(newConnection, (char*)&messagelen, sizeof(int), NULL);	  // send length of chat message
			send(newConnection, message.c_str(), messagelen, NULL);       // send chat message

			Packet test = P_Test;
			send(newConnection, (char*)&test, sizeof(Packet), NULL);
		}
	}

	system("pause");

	return 0;
}
