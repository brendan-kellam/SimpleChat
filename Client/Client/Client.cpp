
#include "Client.h"

// --- constructor ---
Client::Client(string IP, int PORT) {	

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

	addr.sin_addr.s_addr = inet_addr(IP.c_str()); // Connect locally
	addr.sin_port = htons(PORT);				  // Port
	addr.sin_family = AF_INET;					  // IPv4 Socket

	clientptr = this;
}


bool Client::Connect() {
	// set connection socket
	Connection = socket(AF_INET, SOCK_STREAM, NULL);

	// Attempt to establish a connection
	if (connect(Connection, (SOCKADDR*)&addr, sizeofaddr) != 0) {
		// If error occurs...

		MessageBoxA(NULL, "Failed to Connect..", "Error", MB_OK | MB_ICONERROR);
		return false;
	}
	// Create client thread
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientThread, NULL, NULL, NULL);

	return true;
}

bool Client::ProcessPacket(Packet _packettype) {
	switch (_packettype) { // Check for specific packet type

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
		cout << "Unrecognized packet: " << _packettype << endl;
		break;

	}

	return true;
}

bool Client::CloseConnection() {

	if (closesocket(Connection) == SOCKET_ERROR) { // Attempt to close socket.
												   // If error occurs, continue

		if (WSAGetLastError() == WSAENOTSOCK) // If socket is already closed, return true
			return true;
		
		// Handle error
		string ErrorMessage = "Failed to close socket. Winsock Error: " + to_string(WSAGetLastError()) + ".";
		MessageBoxA(NULL, ErrorMessage.c_str(), "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

void Client::ClientThread() { // static method

	Packet packettype; // accepted packet type

	while (true) { // -- Client handler loop --- //

		if (!clientptr->GetPacketType(packettype))  // get packet type			
			break;						 // if error occurs, break

		if (!clientptr->ProcessPacket(packettype)) { // process packet
			break;						  // if error occurs, break
		}

	}

	std::cout << "Lost connection to the server." << std::endl;

	if (clientptr->CloseConnection()) {
		std::cout << "Socket to the server was closed successfuly." << endl;
	}
	else {
		std::cout << "Socket was not able to be closed." << endl;
	}

}


bool Client::SendInt(int _int) {
	int RetnCheck = send(Connection, (char*)&_int, sizeof(int), NULL); // send the int
	return !(RetnCheck == SOCKET_ERROR);							   // Return false if there is a socket error (connection issue)
}

bool Client::GetInt(int &_int) {
	int RetnCheck = recv(Connection, (char*)&_int, sizeof(int), NULL); // get the int
	return !(RetnCheck == SOCKET_ERROR);
}

bool Client::SendPacketType(Packet _packettype) {
	int RetnCheck = send(Connection, (char*)&_packettype, sizeof(Packet), NULL); // send a packet type
	return !(RetnCheck == SOCKET_ERROR);
}

bool Client::GetPacketType(Packet &_packettype) {
	int RetnCheck = recv(Connection, (char*)&_packettype, sizeof(Packet), NULL); // get a packet type
	return  !(RetnCheck == SOCKET_ERROR);
}

bool Client::SendString(string &_string) {

	if (!SendPacketType(P_ChatMessage)) // attepmpt to send chat message packet
		return false;					// failed to send string

	int bufferlen = _string.size(); // get string length

	if (!SendInt(bufferlen)) // send lengh of string 
		return false;

	int RetnCheck = send(Connection, _string.c_str(), bufferlen, NULL); // send string
	return !(RetnCheck == SOCKET_ERROR);
}

bool Client::GetString(string &_string) {
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
