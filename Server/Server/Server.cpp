
#include "Server.h"

/* -- constructor -- */
Server::Server(int PORT, bool BroadcastPublically) {
	std::cout << "server starting...\n";

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

	if (bind(sListen, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) //Bind the address to the socket, if we fail to bind the address..
	{
		std::string ErrorMsg = "Failed to bind the address to our listening socket. Winsock Error:" + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	/* Places the socket in a state such that it is waiting for a incoming connection
	   "SOMAXCONN": Socket outstanding maxiumum connectiosn
	   (The total amount of people that can try and connect at once) */
	if (listen(sListen, SOMAXCONN) == SOCKET_ERROR) { // Handle errors while listening
		std::string ErrorMsg = "Failed to listen on listening socket.";
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}
	
	serverptr = this;

	// Create a new packet sender thread
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)PacketSenderThread, NULL, NULL, NULL);

}

bool Server::ListenForNewConnection() {

	// Socket to hold the client's connection
	SOCKET newConnectionSocket;

	// Accept a new client connection (accept is most likely running on another thread)
	newConnectionSocket = accept(sListen, (SOCKADDR*)&addr, &addrlen);

	// Check if accepting client connection failed 
	if (newConnectionSocket == 0) {
		std::cout << "Failed to accept client connection...\n";
		return false;
	}
	else // If the client connection was accepted
	{

		std::lock_guard<std::mutex> lock(connectionMgr_mutex); // lock connection manager mutex

		size_t conid = connections.size(); // Default new connection id

		// TODO: Optimize!
		if (UnusedConnections > 0)
		{
			// Loop all connections
			for (size_t i = 0; i < connections.size(); i++)
			{
				// find first instance of inactive connection
				if (!connections[i]->activeConnection)
				{
					connections[i]->socket = newConnectionSocket;
					connections[i]->activeConnection = true;
					conid = i;
					UnusedConnections--;
					break;
				}
			}
		}
		else
		{
			// Default case
			std::shared_ptr<Connection> newConnection(new Connection(newConnectionSocket));
			connections.push_back(newConnection);
		}

		std::cout << "Client connected! ID: " << conid << std::endl;


		// create thread for new client
		CreateThread(						   
			NULL, NULL,
			(LPTHREAD_START_ROUTINE)ClientHandlerThread,
			(LPVOID)(conid), 
			NULL, NULL
			);

		
		return true;
	}
}

bool Server::ProcessPacket(int id, PacketType _packettype) {

	switch (_packettype) {
	case PacketType::ChatMessage:
	{

		std::string message;
		if (!GetString(id, message)) // Get chat message from client
			return false;

		// loop all connected clients
		for (size_t i = 0; i < connections.size(); i++) {

			if (!connections[i]->activeConnection) // If connection is not active
				continue;

			if (i == id) 
				continue; 	// Skip originating client

			// Send message to connection i
			SendString((int) i, message);
		}

		std::cout << "Processed chat message packet from user id: " << id << std::endl;
		break;
	}

	// When client request a file from the server
	case PacketType::FileTransferRequestFile:
	{
		std::string FileName;
		if (!GetString(id, FileName))
			return false;


		// attempt to open file to read from
		connections[id]->file.infileStream.open(FileName, std::ios::binary | std::ios::ate);

		// check if file failed to open
		if (!connections[id]->file.infileStream.is_open())
		{
			std::cout << "Client: " << id << " requested file: " << FileName << ". File does not exist" << std::endl;
			std::string errMsg = "Requested file: " + FileName + " does not exist or was not found.";
			SendString(id, errMsg); // Send error message to client

			// No connection issue, so return true
			return true;
		}

		/* -- At this point, the file exists and is ready to be sent -- */

		connections[id]->file.fileName = FileName;						   // Set file name
		connections[id]->file.fileSize = connections[id]->file.infileStream.tellg(); // Get file size
		connections[id]->file.infileStream.seekg(0);					   // Set stream cursor position so no offest exists (start of file)
		connections[id]->file.fileOffset = 0;							   // Update file offest for knowing when we hit EOF

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
		std::cout << "Unrecognized packet: " << _packettype << std::endl;
		break;

	}


	return true;
}

bool Server::SendFileByteBuffer(int id) 
{

	// If end of file is already reached 
	if (connections[id]->file.fileOffset >= connections[id]->file.fileSize)
		return true;

	connections[id]->file.remainingBytes = connections[id]->file.fileSize - connections[id]->file.fileOffset;

	// CASE: # of remaining bytes is greater than our packet size
	if (connections[id]->file.remainingBytes > connections[id]->file.buffersize)
	{

		PS::FileDataBuffer fileData;																	// Create new FileDataBuffer packet structure
		connections[id]->file.infileStream.read(fileData.databuffer, connections[id]->file.buffersize);	// Read from the connection's request file
																											// To the FileDataBuffer buffer
		fileData.size = connections[id]->file.buffersize;												// Set fileData size
		connections[id]->file.fileOffset += connections[id]->file.buffersize;							// Increment the FileTransferData's offset
		connections[id]->pm.Append(fileData.toPacket());												// Appened created packet to packet manager
	} 

	// CASE: # of remaining bytes is less than our packet size
	else
	{
	
		PS::FileDataBuffer fileData;																		// -- Same procedure, but with remaining bytes -- // 
		connections[id]->file.infileStream.read(fileData.databuffer, connections[id]->file.remainingBytes);	
		fileData.size = connections[id]->file.remainingBytes;
		connections[id]->file.fileOffset += connections[id]->file.remainingBytes;
		connections[id]->pm.Append(fileData.toPacket());
	}

	// If we are at EOF
	if (connections[id]->file.fileOffset == connections[id]->file.fileSize)
	{
		Packet EOFPacket(PacketType::FileTransfer_EndOfFile);
		connections[id]->pm.Append(EOFPacket);

		std::cout << std::endl << "File Sent: " << connections[id]->file.fileName << std::endl;
		std::cout << "File size(bytes): " << connections[id]->file.fileSize << std::endl << std::endl;

		// close stream
		connections[id]->file.infileStream.close();
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

	std::cout << "Lost connection to client id: " << id << std::endl; // Prompt when client disconnects
	serverptr->DisconnectClient(id);
	return; 
}

void Server::PacketSenderThread()
{
	while (true)
	{
		// Loop through all connections
		for (size_t i = 0; i < serverptr->connections.size(); i++)
		{
			
			// If a given connection has a pending packet
			if (serverptr->connections[i]->pm.HasPendingPackets())
			{
				// Get packet
				Packet p = serverptr->connections[i]->pm.Retrieve();

				// Attempt to send data 
				if (!serverptr->sendall(i, p.getBuffer(), p.getSize()))
				{
					std::cout << "ERROR: Function(Server::PacketSenderThread) - Failed to send packet to ID: " << i << std::endl;
				}
				delete p.getBuffer(); // Clean up buffer from the packet p
			}
		}
		Sleep(5);
	}
}

/* --- Getters and setters --- */
bool Server::recvall(int id, char* data, int totalbytes) {
	int bytesreceived = 0; // Total # of bytes received

	while (bytesreceived < totalbytes) {

		/* Recieve data from Socket
		Per itteration, offset pointer and the # of bytes to recieve
		*/
		int RetnCheck = recv(connections[id]->socket, data + bytesreceived, totalbytes - bytesreceived, NULL);

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
		int RetnCheck = send(connections[id]->socket, data + bytessent, totalbytes - bytessent, NULL);

		if (RetnCheck == SOCKET_ERROR) // If there was a connection issues
			return false;

		else if (RetnCheck == 0) // If we don't recieve any bytes 
			return false;

		bytessent += RetnCheck;
	}

	return true;
}

bool Server::GetInt32_t(int id, int32_t &_int32_t) {

	if (!recvall(id, (char*)&_int32_t, sizeof(int32_t)))
		return false;	

	_int32_t = ntohl(_int32_t); // Convert long from Host Byte Order to Network byte order
	return true;
}

bool Server::GetPacketType(int id, PacketType &_packettype) {

	int32_t packettype; // Create a local intermediate integer
	if (!GetInt32_t(id, packettype)) // Try to receive packet type..
		return false;			 // If error occurs, return false

	_packettype = (PacketType)packettype; // Case intermediate to type packet
	return true;
}

void Server::SendString(int id, std::string &_string) {

	PS::ChatMessage message(_string);
	connections[id]->pm.Append(message.toPacket());
}

bool Server::GetString(int id, std::string &_string) {
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

void Server::DisconnectClient(int id) // attempt to disconnect client
{
	std::lock_guard<std::mutex> lock(connectionMgr_mutex); // Lock connection manager mutex
	
	if (!connections[id]->activeConnection) // if our client has already been disconnected
	{	
		return;
	}

	connections[id]->pm.Clear();							   // Clear out all remaining packets in queue for this connection
	connections[id]->activeConnection = false;			   // Update connection's activity
	closesocket(connections[id]->socket);				   // close the socket for this connection

	if (id = (connections.size() -1)) // Check if id is last in connections
	{
		connections.pop_back(); // Remove client from vector

		for (size_t i = (connections.size() - 1); i >= 0 && connections.size() > 0; i--)
		{
			if (connections[i]->activeConnection)
				break;

			connections.pop_back();
			UnusedConnections--;
		}
	
	}
	else // If our id is not the last
	{
		UnusedConnections++;
	}
}


