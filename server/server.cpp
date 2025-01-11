#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <string.h>
#include <vector>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include "json.hpp"
#include <sstream>
#include <error.h>
#include <errno.h>
//#include "Room.h"
#include <fstream>
#include <csignal>
#include <set>
#include <mutex>
#include <time.h>
#include <chrono>
#include <random>
#include <optional>
//#include "globals.h"
#include "ClientHandler.h"


// Needed for using JSONs 
using json = nlohmann::json;

// Global serversocket and port 
int serverSocket;
const int PORT = 8080;


// Helper Function to print all clients, used when a new client connet to server, to show all players 
void printAllClients(){
    mutexClientInfoMap.lock();
    std::cout << "Number of clients: " << clientInfoMap.size() << std::endl;
    std::cout << "==============CLIENTS==============" << std::endl;
    for(auto& client: clientInfoMap){
        std::cout <<"ClientFd: "<< client.first << std::endl; // first==key, second==value

        if (client.second.nick.empty()) std::cout <<"Client Nick: " << "UNINITIALIZED" << std::endl;
        else std::cout <<"Client Nick: "<< client.second.nick << std::endl;
        
        std::cout <<"----------------------------------"<< std::endl;
    }
    std::cout << std::endl;
    mutexClientInfoMap.unlock();
}

// Helper function
void printVector(std::vector <int> vec){
    for(auto element: vec){
        std::cout << element << " ";
    }
    std::cout << std::endl;
}

// Function that will clear file rooms.json before server start running and after server close
void clearJsonFIle(const std::string& filePath){
    json emptyData;
    emptyData["rooms"] = json::array();
    std::ofstream ofs(filePath, std::ofstream::trunc);
    if (ofs.is_open()) {
        ofs << emptyData.dump(4) << std::endl;
        ofs.close();
        std::cout << "JSON file cleared: " << filePath << std::endl;
    } else {
        std::cerr << "Failed to open file for clearing: " << filePath << std::endl;
    }
}
// Function to clear rooms.json after Ctrl+C
void shutdownJson(int signal){
    clearJsonFIle("serverJSONs/rooms.json");
    close(serverSocket);
    exit(0);
}


int main() {
    clearJsonFIle("serverJSONs/rooms.json");
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Creating and endpoint for communication as File Descriptor and assigning it to serverSocket. 
    //It uses IPv4 Internet Protocols and TCP protocol
    //Mapping ctrl+c with function shutdownJson
    std::signal(SIGINT, shutdownJson);

    // Checking if serverSocket failed to initialize
    if (serverSocket == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        return 1;
    }


    // Creating a structure that will hold informations about server adress
    sockaddr_in address{};
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = inet_addr("172.18.43.116");
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding serverSocket to the address above
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Starting listening
    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << std::endl;


    //Init both ClientHandler object and RoomHandler object
    // Client handler handle communication with a client
    //Room handler handle game logic
    ClientHandler clientHandler;
    RoomHandler roomHandler(clientHandler);
    while (true) {
        sockaddr_in clientAddr;
        // socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }
        std::cout << "New client connected\n";
        std::cout << "Connecttion from " << inet_ntoa(clientAddr.sin_addr) << " On port:" <<ntohs(clientAddr.sin_port);

        // Helpful unordered_map that will bind clientSocket with a struct ClientInfo that contains a nick
        clientInfoMap[clientSocket] = {};
        //Vector that contain all client sockets
        clientSockets.push_back(clientSocket);
        printAllClients();
        // Starting a new thread for a client, which are handled by clientHandler class
        std::thread([&clientHandler, clientSocket, &roomHandler]() {
            clientHandler.handleClient(clientSocket, roomHandler);
        }).detach();
    }
    //Closing the server socket
    close(serverSocket);
    return 0;
}
