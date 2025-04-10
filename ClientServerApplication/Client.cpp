// Client
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib

using namespace std;

int main() {
	// Initialize Winsock
	WSADATA wsaData;
	int wsResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // Initialize Winsock with version 2.2
	if (wsResult != 0) {
		cerr << "WSAStartup failed: " << wsResult << endl;
		return 1;	
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0); // Create a socket using IPv4 (AF_INET) and TCP (SOCK_STREAM)
	if (clientSocket == INVALID_SOCKET) { // If the socket creation fails, INVALID_SOCKET is returned
		cerr << "Socket creation failed." << endl;
		WSACleanup();
		return 1;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(54000); // Port number. htons converts the port number from host byte order to network byte order.

	// Convert IP address from string to binary
	if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) { // inet_pton function converts the IP address from text to binary form
																// and stores it in serverAddress.sin_addr
		cerr << "Connection failed!" << endl;
		closesocket(clientSocket);
		WSACleanup();
		return -1;
	}

	// Connect to the server
	if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) { // TCP handshake
		// connect function tries to establish a connection to the server
		cerr << "Connection failed!" << endl;
		closesocket(clientSocket);
		WSACleanup();
		return -1;
	}

	cout << "Connected to server!" << endl;

	string message;
	char buffer[1024];

	while (true) {
		cout << "Enter message (type ' exit' to quit): ";
		getline(cin, message); // Read a line of input from the user

		if (message == "exit") {
			break;
		}

		send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0); // send function to send data

		memset(buffer, 0, sizeof(buffer)); // Clear the buffer
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0); // recv function to receive data
		if (bytesReceived > 0) {
			cout << "Echo from server: " << buffer << endl; // Print the received message
		}
		else {
			cout << "Connection closed by server." << endl;
			break;
		}
	}

	// Clean up
	closesocket(clientSocket);
	WSACleanup();
	return 0;

}
