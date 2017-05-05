
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

		Connections[TotalConnections] = newConnection; // add connection to array 
		CreateThread(					// create thread for new client			   
			NULL, NULL,
			(LPTHREAD_START_ROUTINE)ClientHandlerThread,
			(LPVOID)(TotalConnections), // parameter
			NULL, NULL
			);

		string message = "Welcome to the server!"; // create welcome message
		SendString(TotalConnections, message);	   // send message
		TotalConnections++; // increment total # of connections
		
		return true;
	}
}

bool Server::ProcessPacket(int id, Packet _packettype) {

	switch (_packettype) {
	case P_ChatMessage:
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

	default:
		cout << "Unrecognized packet: " << _packettype << endl;
		break;

	}


	return true;
}

void Server::ClientHandlerThread(int id) { // ~~static method~~
	Packet packettype;
	while (true) {

		if (!serverptr->GetPacketType(id, packettype)) // Get packet type from client
			break;							// If error occurs, break from loop

		if (!serverptr->ProcessPacket(id, packettype)) // Process packet
			break;							// If error occurs, break from loop
	}

	std::cout << "Lost connection to client id: " << id << endl; // Prompt when client disconnects
	closesocket(serverptr->Connections[id]); //when done with a client, close the socket	
}

/* --- Getters and setters --- */

bool Server::SendInt(int id, int _int) {
	int RetnCheck = send(Connections[id], (char*)&_int, sizeof(int), NULL); // send the int
	return !(RetnCheck == SOCKET_ERROR);							   // Return false if there is a socket error (connection issue)
}

bool Server::GetInt(int id, int &_int) {
	int RetnCheck = recv(Connections[id], (char*)&_int, sizeof(int), NULL); // get the int
	return !(RetnCheck == SOCKET_ERROR);							   // Return false if there is a socket error (connection issue)
}

bool Server::SendPacketType(int id, Packet _packettype) {
	int RetnCheck = send(Connections[id], (char*)&_packettype, sizeof(Packet), NULL); // send a packet type
	return !(RetnCheck == SOCKET_ERROR);
}

bool Server::GetPacketType(int id, Packet &_packettype) {
	int RetnCheck = recv(Connections[id], (char*)&_packettype, sizeof(Packet), NULL); // get a packet type
	return  !(RetnCheck == SOCKET_ERROR);
}

bool Server::SendString(int id, string &_string) {

	if (!SendPacketType(id, P_ChatMessage)) // attepmpt to send chat message packet
		return false;					// failed to send string

	int bufferlen = _string.size(); // get string length

	if (!SendInt(id, bufferlen)) // send lengh of string 
		return false;

	int RetnCheck = send(Connections[id], _string.c_str(), bufferlen, NULL); // send string
	return !(RetnCheck == SOCKET_ERROR);
}

bool Server::GetString(int id, string &_string) {
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

/* -------------------------- */