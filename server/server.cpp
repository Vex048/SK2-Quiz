#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <string.h>
#include <vector>
#include <unordered_map>

#include <sstream>
#include <error.h>
#include <errno.h>

const int PORT = 8080;
std::vector<int> clientSockets;

struct clientInfo{
    std::string nick;
    // later on we can add more info about client such as score, current_quiz, etc.
};
std::unordered_map<int, clientInfo> clientInfoMap;

void printAllClients(){
    std::cout << "Number of clients: " << clientInfoMap.size() << std::endl;
    std::cout << "==============CLIENTS==============" << std::endl;
    for(auto& client: clientInfoMap){
        std::cout <<"ClientFd: "<< client.first << std::endl; // first==key, second==value

        if (client.second.nick.empty()) std::cout <<"Client Nick: " << "UNINITIALIZED" << std::endl;
        else std::cout <<"Client Nick: "<< client.second.nick << std::endl;
        
        std::cout <<"----------------------------------"<< std::endl;
    }
    std::cout << std::endl;
}

std::vector<std::string> splitMessage(const std::string& message, char delimiter) {
    std::vector<std::string> parts;
    std::istringstream stream(message);
    std::string part;

    while (std::getline(stream, part, delimiter)) {
        parts.push_back(part);
    }

    return parts;
}
void disconnectClient(int clientFd){
    close(clientFd);
    clientInfoMap.erase(clientFd);
}

int readMessage(int clientFd, char * buffer,int bufSize){
    int n = recv(clientFd,buffer,bufSize,0);
    if (n<=0){
        if(n<0) error(0,errno,"Error on read from client %d",clientFd);

        disconnectClient(clientFd);
        std::cout << "Client disconnected gracefully\n";
        return -1;
    }

    std::vector<std::string> parts = splitMessage(buffer, '|');
    if(parts.size()>=2){
        std::string operation = parts[0];
        std::string message = parts[1];
        
        if (operation == "NICK") {
            clientInfoMap[clientFd].nick = message;
            std::cout << "New client's username: "<< clientInfoMap[clientFd].nick << std::endl;
            printAllClients();
        } else if (operation == "JOIN") {
            std::cout << "Client " << clientInfoMap[clientFd].nick << " joined room number: "<< message << std::endl;
        } else {
            std::cout << "Unknown operation: " << operation << std::endl;
        }
    }
    // std::cout << "Received: " << buffer << std::endl;
    return n;
}

void handleClient(int clientSocket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int n = readMessage(clientSocket,buffer,sizeof(buffer));
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
        clientInfoMap[clientSocket] = {};
        clientSockets.push_back(clientSocket);
        std::thread(handleClient, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}
