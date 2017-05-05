
#include "Client.h"



int main(int argc, char** argv) {	

	Client myClient("127.0.0.1", 1111); // Create new client

	if (!myClient.Connect()) { // If client fails to connect
		cout << "Failed to connect to server..." << endl;
		system("pause");
		return 1;
	}


	// -- Outgoing message handling -- //
	string input;
	while (true) {

		getline(cin, input); // Get line passed in from user
		if (!myClient.SendString(input)) // If we fail to send user string, break from loop
			break;

		Sleep(10);
	}

	system("pause");
	return 0;
}