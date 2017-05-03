/**
Server
***/

// includes  

// Allows to access functions
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <iostream>

#define HOST_IP "127.0.0.1"
#define HOST_PORT 1111
#define MAX_CONNECTIONS 100

using namespace std;

// Array of connections
SOCKET Connections[MAX_CONNECTIONS];
int ConnectionCounter = 0;


void ClientHandlerThread(int index) {

	// Client's respective message buffer
	char buffer[256];

	while (true) {
		// Recive a new message from the current client 
		recv(Connections[index], buffer, sizeof(buffer), NULL);
		
		// Loop all connected clients and send message
		for (int i = 0; i < ConnectionCounter; i++) {

			// Skip originating client
			if (i == index) continue;

			// Send message to client
			send(Connections[i], buffer, sizeof(buffer), NULL);
		}

	}

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

			// Create a buffer with message
			char message[256] = "Welcome to the server!";

			// Send new message to client
			send(newConnection, message, sizeof(message), NULL);
			
			// Add created connection to array
			Connections[i] = newConnection;
			
			// Increment the number of connections
			ConnectionCounter++;

			// Create a new thread for the client
			CreateThread(NULL, NULL,
				(LPTHREAD_START_ROUTINE)ClientHandlerThread,
				(LPVOID)(i), // parameter
				NULL, NULL
				);

		}
	}

	system("pause");

	return 0;
}
