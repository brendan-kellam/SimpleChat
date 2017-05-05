
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

bool Client::recvall(char* data, int totalbytes) {
	int bytesreceived = 0; // Total # of bytes received

	while (bytesreceived < totalbytes) {
		
		/* Recieve data from Socket
		   Per itteration, offset pointer and the # of bytes to recieve
		*/
		int RetnCheck = recv(Connection, data + bytesreceived, totalbytes - bytesreceived, NULL);
	
		if (RetnCheck == SOCKET_ERROR) // If there was a connection issues
			return false;

		else if (RetnCheck == 0) // If we don't recieve any bytes 
			return false;
		
		bytesreceived += RetnCheck; // Add to total bytes recieved
	}

	return true;
}

bool Client::sendall(char* data, int totalbytes) {

	int bytessent = 0; // Total # of bytes sent
	
	while (bytessent < totalbytes) {
		// Send data over socket
		int RetnCheck = send(Connection, data + bytessent, totalbytes - bytessent, NULL);

		if (RetnCheck == SOCKET_ERROR) // If there was a connection issues
			return false;

		else if (RetnCheck == 0) // If we don't recieve any bytes 
			return false;

		bytessent += RetnCheck;
	}

	return true;
}

bool Client::SendInt(int _int) {
	return sendall((char*)&_int, sizeof(int));
}

bool Client::GetInt(int &_int) {
	return recvall((char*)&_int, sizeof(int));
}

bool Client::SendPacketType(Packet _packettype) {
	return sendall((char*)&_packettype, sizeof(Packet));
}

bool Client::GetPacketType(Packet &_packettype) {
	return recvall((char*)&_packettype, sizeof(Packet));
}

bool Client::SendString(string &_string) {

	if (!SendPacketType(P_ChatMessage)) // attepmpt to send chat message packet
		return false;					// failed to send string

	int bufferlen = _string.size(); // get string length

	if (!SendInt(bufferlen)) // send lengh of string 
		return false;

	return sendall((char*) _string.c_str(), bufferlen);
}

bool Client::GetString(string &_string) {
	int bufferlen;

	if (!GetInt(bufferlen)) // Get bufferlength
		return false;

	char* buffer = new char[bufferlen + 1]; 				   // create a new recieve buffer
	buffer[bufferlen] = '\0';								   // set last character to null terminator

	// If there is a connection issue while 
	if (!recvall(buffer, bufferlen)) {
		delete[] buffer; // dealocate buffer before returning
		return false; 
	}

	_string = buffer; // set return string
	delete[] buffer; // dealocate buffer

	return true;
}
