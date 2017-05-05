/**
Server
***/

// includes  
#include "Server.h"



int main(int argc, char** argv) {
	
	// Create a new local server on port 1111
	Server myServer(1111);

	// accept new connection up to 100 times
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		myServer.ListenForNewConnection();
	}


	return 0;
}
