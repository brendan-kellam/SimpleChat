/** Client

Communication pipeline:
	1. Initially send integer that will indicate size (in bytes) of incomming packet
	2. Send our packet
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

void ClientThread() {

	// Length of incomming buffer
	int bufferlen;
	
	while (true) {
		
		// recieve buffer length
		recv(Connection, (char*)&bufferlen, sizeof(int), NULL);
	
		// create a new recieve buffer
		char* buffer = new char[bufferlen+1];
		 // Set last char to be null terminator

		// recieve message from server
		recv(Connection, buffer, bufferlen, NULL);
		buffer[bufferlen] = '\0';
	
		// print out message
		cout << buffer << endl;

		// dealocate memory
		delete[] buffer;
	}
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




	string buffer;

	while (true) {

		// Get message from client
		getline(cin, buffer);
		int bufferlen = buffer.size();
		send(Connection, (char*)&bufferlen, sizeof(int), NULL);
		
		// Send message to server
		send(Connection, buffer.c_str(), bufferlen, NULL);
		Sleep(10);
	}


	system("pause");
	return 0;
}