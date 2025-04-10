// Server
#include <iostream>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "ws2_32.lib") // link with ws2_32.lib
								   // Tells the compiler to link the Winsock library
#pragma comment(lib, "iphlpapi.lib") // link with iphlpapi.lib
									 // Tells the compiler to linkt the IP Helper library

using namespace std;

// Function to get the MAC address of the client (from the ARP table)
void getMACAddress(const char* ipAddress) {
	struct in_addr ipAddr;
	if (inet_pton(AF_INET, ipAddress, &ipAddr) != 1) {
		cerr << "Invalid IP address: " << ipAddress << endl;
		return;
	}

	// Get the MAC address using ARP
	UCHAR macAddr[6];
	DWORD dwRetVal;
	DWORD dwSize = sizeof(macAddr);

	dwRetVal = SendARP((IPAddr)ipAddr.s_addr, 0, macAddr, &dwSize); // SendARP function sends an ARP request to the specified IP address
	if (dwRetVal == NO_ERROR) {
		cout << "\tMAC Address: ";
		for (int i = 0; i < 6; i++) {
			cout << hex << int(macAddr[i]);
			if (i != 5) {
				cout << ":";
			}
		}
		cout << "\n";
	}
	else {
		cerr << "Error getting MAC address: " << WSAGetLastError() << endl; // If SendARP fails, print the error
	}
}

void handleClient(SOCKET clientSocket) {
	char buffer[1024];

	// Get the client address
	sockaddr_in clientAddress;
	int clientAddrSize = sizeof(clientAddress);
	getpeername(clientSocket, (struct sockaddr*)&clientAddress, &clientAddrSize); // getpeername function retrieves the address of the connected peer
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, INET_ADDRSTRLEN); // inet_ntop function converts the binary IP address to string form
	
	cout << "Client connected\n\t" << "IP: " << clientIP << endl;

	getMACAddress(clientIP); // Only works in local network
	
	while (true) {
		memset(buffer, 0, sizeof(buffer));
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0); // recv function to receive data
		if (bytesReceived <= 0) { // If bytesReceived is 0 or less, the connection has been closed or an error occurred
			cout << "Client disconnected." << endl;
			break;
		}
		cout << "Message from client " << clientIP << " : " << buffer << endl; // Print the received message

		// Echo the message back to the client
		send(clientSocket, buffer, bytesReceived, 0);
	}
	closesocket(clientSocket);
}

int main() {
	// Initialize Winsock
	WSADATA wsaData;
	int wsResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // MAKEWORD(2, 2) specifies the version of Winsock to use (2.2)
	if (wsResult != 0) {
		cerr << "WSAStartup failed: " << wsResult << endl;
		return 1;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Create a socket using IPv4 (AF_INET) and TCP (SOCK_STREAM)
	if (serverSocket == INVALID_SOCKET) { // If the socket creation fails, INVALID_SOCKET is returned
		cerr << "Socket creation failed." << endl;
		WSACleanup();
		return 1;
	}

	// Binding the socket to an address
	sockaddr_in serverAddress; // sockaddr_in is a structure that contains an internet address (IP and port)
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(54000); // Port number. htons converts the port number from host byte order to network byte order. 
										   // Big-endian format is used for network byte order.
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Listen on any available interface

	if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) { // bind function binds the socket to the address
		cerr << "Bind failed." << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	if (listen(serverSocket, 5) == SOCKET_ERROR) { // listening for incoming connections. The second parameter specifies the maximum length of the queue for pending connections.
		cerr << "Listen failed." << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	cout << "Server is listening on port 54000..." << endl;

	while (true) {
		SOCKET clientSocket = accept(serverSocket, nullptr, nullptr); // accept function waits for an incoming connection. It returns a new socket for the connection.
		if (clientSocket == INVALID_SOCKET){
			cerr << "Failed to accept connection." << endl;
			continue;
		}
		cout << "New client connected." << endl;
		thread(handleClient, clientSocket).detach(); // Handle client in a separate thread. std::thread is used to create a new thread. 
		//detach() allows the thread to run independently. No need to join it later.
	}

	// Cleanup
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}