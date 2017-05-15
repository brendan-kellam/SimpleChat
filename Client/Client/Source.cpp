
#include "Client.h"



int main(int argc, char** argv) {	

	Client myClient("127.0.0.1", 1111); // Create new client

	if (!myClient.Connect()) { // If client fails to connect
		std::cout << "Failed to connect to server..." << std::endl;
		system("pause");
		return 1;
	}

	//myClient.RequestFile("image.jpg");

	// -- Outgoing message handling -- //
	std::string input;
	while (true) {

		getline(std::cin, input); // Get line passed in from user
		if (!myClient.SendString(input)) // If we fail to send user string, break from loop
			break;

		Sleep(10);
	}

	system("pause");
	return 0;
}