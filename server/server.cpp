#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

const int PORT = 8080;
std::vector<int> clientSockets;

void handleClient(int clientSocket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            std::cout << "Client disconnected\n";
            close(clientSocket);
            return;
        }
        std::cout << "Received: " << buffer << std::endl;

        std::string response = "QUESTION|What is 2+2?";
        send(clientSocket, response.c_str(), response.size(), 0);
    }
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }
        std::cout << "New client connected\n";
        std::cout << "Connecttion from " << inet_ntoa(clientAddr.sin_addr) << " On port:" <<ntohs(clientAddr.sin_port);
        clientSockets.push_back(clientSocket);
        std::thread(handleClient, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}
