
#include "Server.h"

/* -- constructor -- */
Server::Server(int PORT, bool BroadcastPublically) {
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

	if (BroadcastPublically)
		addr.sin_addr.s_addr = htonl(INADDR_ANY);	// Broadcast publically (converts host to netowrk byte order) [Host to Network Long]
	else 
		addr.sin_addr.s_addr = inet_addr(LOCAL_IP); // Broadcast locally
	
	
	addr.sin_port = htons(PORT); // Port
	addr.sin_family = AF_INET; // IPv4 Socket


							   // Create socket to listen for new connections
	sListen = socket(AF_INET, SOCK_STREAM, NULL);

	// Bind the address to the socket
	if (bind(sListen, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) { // Handle errors while binding
		string ErrorMsg = "Failed to bind the address to our listening socket";
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	/* Places the socket in a state such that it is waiting for a incoming connection
	   "SOMAXCONN": Socket outstanding maxiumum connectiosn
	   (The total amount of people that can try and connect at once) */
	if (listen(sListen, SOMAXCONN) == SOCKET_ERROR) { // Handle errors while listening
		string ErrorMsg = "Failed to listen on listening socket.";
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}
	
	serverptr = this;
}

bool Server::ListenForNewConnection() {

	// Socket to hold the client's connection
	SOCKET newConnection;

	// Accept a new client connection (accept is most likely running on another thread)
	newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen);

	// Check if accepting client connection failed 
	if (newConnection == 0) {
		cout << "Failed to accept client connection...\n";
		return false;
	}

	// If the client connection was accepted
	else {
		cout << "Client connected!\n";

		connections[TotalConnections].socket = newConnection; // add connection to array 
		CreateThread(					// create thread for new client			   
			NULL, NULL,
			(LPTHREAD_START_ROUTINE)ClientHandlerThread,
			(LPVOID)(TotalConnections), // parameter
			NULL, NULL
			);

		//string message = "Welcome to the server!"; // create welcome message
		//SendString(TotalConnections, message);	   // send message
		TotalConnections++; // increment total # of connections
		
		return true;
	}
}

bool Server::ProcessPacket(int id, PacketType _packettype) {

	switch (_packettype) {
	case PacketType::ChatMessage:
	{

		string message;
		if (!GetString(id, message)) // Get chat message from client
			return false;

		// loop all connected clients
		for (int i = 0; i < TotalConnections; i++) {

			if (i == id) continue; 	// Skip originating client

			if (!SendString(i, message)) {													  // Send message to connection @ index i
				cout << "Failed to send message from id: " << id << ", to id: " << i << endl; // Error message
			}
		}

		cout << "Processed chat message packet from user id: " << id << endl;
		break;
	}

	// When client request a file from the server
	case PacketType::FileTransferRequestFile:
	{
		string FileName;
		if (!GetString(id, FileName))
			return false;


		// attempt to open file to read from
		connections[id].file.infileStream.open(FileName, std::ios::binary | std::ios::ate);

		// check if file failed to open
		if (!connections[id].file.infileStream.is_open())
		{
			cout << "Client: " << id << " requested file: " << FileName << ". File does not exist" << endl;
			string errMsg = "Requested file: " + FileName + " does not exist or was not found.";
			if (!SendString(id, errMsg)) // Send error message to client
				return false;

			// No connection issue, so return true
			return true;
		}

		/* -- At this point, the file exists and is ready to be sent -- */

		connections[id].file.fileName = FileName;						   // Set file name
		connections[id].file.fileSize = connections[id].file.infileStream.tellg(); // Get file size
		connections[id].file.infileStream.seekg(0);					   // Set stream cursor position so no offest exists (start of file)
		connections[id].file.fileOffset = 0;							   // Update file offest for knowing when we hit EOF

		/* -- Ready to send first byte buffer -- */
		if (!SendFileByteBuffer(id))
			return false;

		break;
	}

	case PacketType::FileTransferRequestNextBuffer:
	{
		if (!SendFileByteBuffer(id))
			return false;

		break;
	}

	default:
		cout << "Unrecognized packet: " << _packettype << endl;
		break;

	}


	return true;
}

bool Server::SendFileByteBuffer(int id) 
{

	// If end of file is already reached 
	if (connections[id].file.fileOffset >= connections[id].file.fileSize)
		return true;

	// Send packet type for file byte buffer transfer
	if (!SendPacketType(id, PacketType::FileTransferByteBuffer))
		return false;

	connections[id].file.remainingBytes = connections[id].file.fileSize - connections[id].file.fileOffset;

	// If # of remaining bytes is greater than our packet size
	if (connections[id].file.remainingBytes > connections[id].file.buffersize)
	{
		// Read from filestream into buffer
		connections[id].file.infileStream.read(connections[id].file.buffer, connections[id].file.buffersize);

		// Send int of buffer size
		if (!SendInt32_t(id, connections[id].file.buffersize))
			return false;

		// Send bytes
		if (!sendall(id, connections[id].file.buffer, connections[id].file.buffersize))
			return false;
	
		// increment offset
		connections[id].file.fileOffset += connections[id].file.buffersize; 
	} 
	else
	{
		connections[id].file.infileStream.read(connections[id].file.buffer, connections[id].file.remainingBytes);
		if (!SendInt32_t(id, connections[id].file.remainingBytes))
			return false;

		if (!sendall(id, connections[id].file.buffer, connections[id].file.remainingBytes))
			return false;

		connections[id].file.fileOffset += connections[id].file.remainingBytes;
	}

	// If we are at EOF
	if (connections[id].file.fileOffset == connections[id].file.fileSize)
	{
		if (!SendPacketType(id, PacketType::FileTransfer_EndOfFile))
			return false;

		cout << endl << "File Sent: " << connections[id].file.fileName << endl;
		cout << "File size(bytes): " << connections[id].file.fileSize << endl << endl;

		// close stream
		connections[id].file.infileStream.close();
	}

	return true;
}

void Server::ClientHandlerThread(int id) { // ~~static method~~
	PacketType packettype;
	while (true) {

		if (!serverptr->GetPacketType(id, packettype)) // Get packet type from client
			break;							// If error occurs, break from loop

		if (!serverptr->ProcessPacket(id, packettype)) // Process packet
			break;							// If error occurs, break from loop
	}

	std::cout << "Lost connection to client id: " << id << endl; // Prompt when client disconnects
	closesocket(serverptr->connections[id].socket); //when done with a client, close the socket	
}

/* --- Getters and setters --- */
bool Server::recvall(int id, char* data, int totalbytes) {
	int bytesreceived = 0; // Total # of bytes received

	while (bytesreceived < totalbytes) {

		/* Recieve data from Socket
		Per itteration, offset pointer and the # of bytes to recieve
		*/
		int RetnCheck = recv(connections[id].socket, data + bytesreceived, totalbytes - bytesreceived, NULL);

		if (RetnCheck == SOCKET_ERROR) // If there was a connection issues
			return false;

		else if (RetnCheck == 0) // If we don't recieve any bytes 
			return false;

		bytesreceived += RetnCheck; // Add to total bytes recieved
	}

	return true;
}

bool Server::sendall(int id, char* data, int totalbytes) {

	int bytessent = 0; // Total # of bytes sent

	while (bytessent < totalbytes) {
		// Send data over socket
		int RetnCheck = send(connections[id].socket, data + bytessent, totalbytes - bytessent, NULL);

		if (RetnCheck == SOCKET_ERROR) // If there was a connection issues
			return false;

		else if (RetnCheck == 0) // If we don't recieve any bytes 
			return false;

		bytessent += RetnCheck;
	}

	return true;
}

bool Server::SendInt32_t(int id, int32_t _int32_t) {
	_int32_t = htonl(_int32_t); // Convert long from Host Byte Order to Network byte order
	return sendall(id, (char*)&_int32_t, sizeof(int32_t));
}

bool Server::GetInt32_t(int id, int32_t &_int32_t) {

	if (!recvall(id, (char*)&_int32_t, sizeof(int32_t)))
		return false;	

	_int32_t = ntohl(_int32_t); // Convert long from Host Byte Order to Network byte order
	return true;
}

bool Server::SendPacketType(int id, PacketType _packettype) {
	return SendInt32_t(id, (int32_t) _packettype);
}

bool Server::GetPacketType(int id, PacketType &_packettype) {

	int32_t packettype; // Create a local intermediate integer
	if (!GetInt32_t(id, packettype)) // Try to receive packet type..
		return false;			 // If error occurs, return false

	_packettype = (PacketType)packettype; // Case intermediate to type packet
	return true;
}

bool Server::SendString(int id, string &_string) {

	if (!SendPacketType(id, PacketType::ChatMessage)) // attepmpt to send chat message packet
		return false;					// failed to send string

	int bufferlen = _string.size(); // get string length

	if (!SendInt32_t(id, bufferlen)) // send lengh of string 
		return false;

	return sendall(id, (char*)_string.c_str(), bufferlen);
}

bool Server::GetString(int id, string &_string) {
	int bufferlen;

	if (!GetInt32_t(id, bufferlen)) // Get bufferlength
		return false;			// If error occurs

	char* buffer = new char[bufferlen + 1]; 				   // create a new recieve buffer
	buffer[bufferlen] = '\0';								   // set last character to null terminator
	
	// Handle network error
	if (!recvall(id, buffer, bufferlen)) {
		delete[] buffer;
		return false;
	}
	_string = buffer; // set return string
	delete[] buffer; // dealocate buffer

	return true;
}

/* -------------------------- */