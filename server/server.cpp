#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <string.h>
#include <vector>
#include <sstream>
#include <error.h>
#include <errno.h>

const int PORT = 8080;
std::vector<int> clientSockets;


std::vector<std::string> splitMessage(const std::string& message, char delimiter) {
    std::vector<std::string> parts;
    std::istringstream stream(message);
    std::string part;

    while (std::getline(stream, part, delimiter)) {
        parts.push_back(part);
    }

    return parts;
}

int readMessage(int clientFd, char * buffer,int bufSize){
    int n = recv(clientFd,buffer,bufSize,0);
    if (n<0){
        error(0,errno,"Error on read from client %d",clientFd);
        std::cout << "Client disconnected gracefully\n";
        close(clientFd);
        return -1;
        
    }
    else if (n==0){
        std::cout << "Client disconnected gracefully\n";
        close(clientFd);
        return 0;
    }

    std::vector<std::string> parts = splitMessage(buffer, '|');
    if(parts.size()>=2){
        std::string operation = parts[0];
        std::string message = parts[1];
        std::cout << operation << ": "<< message << std::endl;
    }
    std::cout << "Received: " << buffer << std::endl;
    return n;
}

void handleClient(int clientSocket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int n =readMessage(clientSocket,buffer,sizeof(buffer));
        if(n<=0) return;
        
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
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }
        std::cout << "New client connected\n";
        clientSockets.push_back(clientSocket);
        std::thread(handleClient, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}
