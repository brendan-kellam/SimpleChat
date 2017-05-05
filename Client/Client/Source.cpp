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
	P_ChatMessage
};

bool SendInt(int _int) {
	int RetnCheck = send(Connection, (char*)&_int, sizeof(int), NULL); // send the int
	return !(RetnCheck == SOCKET_ERROR);							   // Return false if there is a socket error (connection issue)
} 

bool GetInt(int &_int) {
	int RetnCheck = recv(Connection, (char*)&_int, sizeof(int), NULL); // get the int
	return !(RetnCheck == SOCKET_ERROR);
}

bool SendPacketType(Packet _packettype) {
	int RetnCheck = send(Connection, (char*)&_packettype, sizeof(Packet), NULL); // send a packet type
	return !(RetnCheck == SOCKET_ERROR);
}

bool GetPacketType(Packet &_packettype) {
	int RetnCheck = recv(Connection, (char*)&_packettype, sizeof(Packet), NULL); // get a packet type
	return  !(RetnCheck == SOCKET_ERROR);
}

bool SendString(string &_string) {

	if (!SendPacketType(P_ChatMessage)) // attepmpt to send chat message packet
		return false;					// failed to send string

	int bufferlen = _string.size(); // get string length

	if (!SendInt(bufferlen)) // send lengh of string 
		return false;

	int RetnCheck = send(Connection, _string.c_str(), bufferlen, NULL); // send string
	return !(RetnCheck == SOCKET_ERROR);
}

bool GetString(string &_string) {
	int bufferlen;

	if (!GetInt(bufferlen)) // Get bufferlength
		return false;

	char* buffer = new char[bufferlen + 1]; 				   // create a new recieve buffer
	buffer[bufferlen] = '\0';								   // set last character to null terminator
	int RetnCheck = recv(Connection, buffer, bufferlen, NULL); // accept message

	_string = buffer; // set return string
	delete[] buffer; // dealocate buffer

	return !(RetnCheck == SOCKET_ERROR);
}

bool ProcessPacket(Packet packettype) {

	switch (packettype) { // Check for specific packet type

		case P_ChatMessage:
		{

			string message;
			if (!GetString(message)) // attempt to get the message string
				return false;

			cout << "Message: " << message << endl; // display message to user
			break;
		}

		// handle unrecognized packets
		default:
			cout << "Unrecognized packet: " << packettype << endl;
			break;

	}


	return true;
}

void ClientThread() {
	
	Packet packettype; // accepted packet type

	while (true) { // -- Client handler loop --- //
		
		if (!GetPacketType(packettype))  // get packet type			
			break;						 // if error occurs, break

		if (!ProcessPacket(packettype)) { // process packet
			break;						  // if error occurs, break
		}
		
	}

	std::cout << "Lost connection to the server." << std::endl;
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
	addr.sin_port = htons(HOST_PORT);			// Port
	addr.sin_family = AF_INET;					// IPv4 Socket


	// set connection socket
	Connection = socket(AF_INET, SOCK_STREAM, NULL);

	// Attempt to establish a connection
	if (connect(Connection, (SOCKADDR*) &addr, addrlen) != 0){
		// If error occurs...

		MessageBoxA(NULL, "Failed to Connect..", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	cout << "Connected!\n";

	// Create client thread
	CreateThread(NULL, NULL,
		(LPTHREAD_START_ROUTINE)ClientThread,
		NULL, NULL, NULL
	);


	// -- Outgoing message handling -- //

	string input;
	while (true) {

		getline(cin, input); // Get line passed in from user
		if (!SendString(input)) // If we fail to send user string, break from loop
			break;

		Sleep(10);
	}

	system("pause");
	return 0;
}