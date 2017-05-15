
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

bool Client::ProcessPacket(PacketType _packettype) {
	switch (_packettype) { // Check for specific packet type

	// If chat message is recieved
	case PacketType::ChatMessage:
	{

		string message;
		if (!GetString(message)) // attempt to get the message string
			return false;

		cout << "Message: " << message << endl; // display message to user
		break;
	}

	// If we are receiving a byte buffer from server
	case PacketType::FileTransferByteBuffer:
	{
		int32_t buffersize;
		if (!GetInt32_t(buffersize))
			return false;
		
		if (!recvall(file.buffer, buffersize))
			return false;
		
		file.outfileStream.write(file.buffer, buffersize);
		file.bytesWritten += buffersize;
		cout << "Recieved byte buffer for file transfer of size: " << buffersize << endl;
		
		// Request next file packet
		SendPacketType(PacketType::FileTransferRequestNextBuffer);
		break;
	}

	// If we are receiving a EOF flag from server
	case PacketType::FileTransfer_EndOfFile:
	{
		cout << "File transfer completed. File received." << endl;
		cout << "File Name: " << file.fileName << endl;
		cout << "File Size(bytes): " << file.bytesWritten << endl;
		file.bytesWritten = 0;
		file.outfileStream.close();
		break;
	}


	// handle unrecognized packets
	default:
		cout << "ERROR: Function(Client::ProcessPacket) - Unrecognized packet: " << _packettype << endl;
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

	PacketType packettype; // accepted packet type

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

bool Client::RequestFile(string FileName) 
{
	/* FLOW */
	// Open file to write to
	// Ensure file was opened correctly
	// Send packet to server to request file
	// Send name of file to server

	file.outfileStream.open(FileName, std::ios::binary);
	file.fileName = FileName;
	file.bytesWritten = 0;

	if (!file.outfileStream.is_open())
	{
		std::cout << "ERROR: Function(Client::RequestFile) - Unable to open file: " << FileName << endl;
		return false;
	}

	std::cout << "Requesting file from server:  " << FileName << std::endl;

	if (!SendPacketType(PacketType::FileTransferRequestFile))
		return false;

	if (!SendString(FileName, false))
		return false;

	return true;
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

bool Client::SendInt32_t(int32_t _int32_t) {
	_int32_t = htonl(_int32_t);  // Convert long from Host Byte Order to Network byte order
	return sendall((char*)&_int32_t, sizeof(int32_t));
}

bool Client::GetInt32_t(int32_t &_int32_t) {

	if (!recvall((char*)&_int32_t, sizeof(int32_t)))
		return false;

	_int32_t = ntohl(_int32_t); 
	return true;
}

bool Client::SendPacketType(PacketType _packettype) {
	return SendInt32_t((int32_t) _packettype); // attempt to send the packet
}

bool Client::GetPacketType(PacketType &_packettype) {

	int32_t packettype; // Create a local intermediate integer
	if (!GetInt32_t(packettype)) // Try to receive packet type..
		return false;			 // If error occurs, return false

	_packettype = (PacketType) packettype; // Case intermediate to type packet
	return true;
}

bool Client::SendString(string &_string, bool IncludePacketType) {
	
	// If we choose to include the packettype
	if (IncludePacketType)
	{
		if (!SendPacketType(PacketType::ChatMessage)) // attepmpt to send chat message packet
			return false;					// failed to send string
	}

	int bufferlen = _string.size(); // get string length

	if (!SendInt32_t(bufferlen)) // send lengh of string 
		return false;

	return sendall((char*) _string.c_str(), bufferlen);
}

bool Client::GetString(string &_string) {
	int bufferlen;

	if (!GetInt32_t(bufferlen)) // Get bufferlength
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
