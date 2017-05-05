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
	P_ChatMessage
};

bool SendInt(int id, int _int) {
	int RetnCheck = send(Connections[id], (char*)&_int, sizeof(int), NULL); // send the int
	return !(RetnCheck == SOCKET_ERROR);							   // Return false if there is a socket error (connection issue)
}

bool GetInt(int id, int &_int) {
	int RetnCheck = recv(Connections[id], (char*)&_int, sizeof(int), NULL); // get the int
	return !(RetnCheck == SOCKET_ERROR);							   // Return false if there is a socket error (connection issue)
}

bool SendPacketType(int id, Packet _packettype) {
	int RetnCheck = send(Connections[id], (char*)&_packettype, sizeof(Packet), NULL); // send a packet type
	return !(RetnCheck == SOCKET_ERROR);
}

bool GetPacketType(int id, Packet &_packettype) {
	int RetnCheck = recv(Connections[id], (char*)&_packettype, sizeof(Packet), NULL); // get a packet type
	return  !(RetnCheck == SOCKET_ERROR);
}

bool SendString(int id, string &_string) {

	if (!SendPacketType(id, P_ChatMessage)) // attepmpt to send chat message packet
		return false;					// failed to send string

	int bufferlen = _string.size(); // get string length

	if (!SendInt(id, bufferlen)) // send lengh of string 
		return false;

	int RetnCheck = send(Connections[id], _string.c_str(), bufferlen, NULL); // send string
	return !(RetnCheck == SOCKET_ERROR);
}

bool GetString(int id, string &_string) {
	int bufferlen;

	if (!GetInt(id, bufferlen)) // Get bufferlength
		return false;			// If error occurs

	char* buffer = new char[bufferlen + 1]; 				   // create a new recieve buffer
	buffer[bufferlen] = '\0';								   // set last character to null terminator
	int RetnCheck = recv(Connections[id], buffer, bufferlen, NULL); // accept message

	_string = buffer; // set return string
	delete[] buffer; // dealocate buffer

	return !(RetnCheck == SOCKET_ERROR);
}

bool ProcessPacket(int id, Packet packettype) {

	switch (packettype) {
	case P_ChatMessage:
	{

		string message;
		if (!GetString(id, message)) // Get chat message from client
			return false;

		// loop all connected clients
		for (int i = 0; i < ConnectionCounter; i++) {

			if (i == id) continue; 	// Skip originating client

			if (!SendString(i, message)) {													  // Send message to connection @ index i
				cout << "Failed to send message from id: " << id << ", to id: " << i << endl; // Error message
			}
		} 

		cout << "Processed chat message packet from user id: " << id << endl;
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
			
		if (!GetPacketType(id, packettype)) // Get packet type from client
			break;							// If error occurs, break from loop

		if (!ProcessPacket(id, packettype)) // Process packet
			break;							// If error occurs, break from loop
	}

	std::cout << "Lost connection to client id: " << id << endl; // Prompt when client disconnects
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

	
			string message = "Welcome to the server!"; // create welcome message
			SendString(id, message);					   // send message
		}
	}

	system("pause");

	return 0;
}
