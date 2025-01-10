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
using json = nlohmann::json;

// std::mutex mutexRooms;
// std::mutex mutexClientInfoMap;
// std::mutex mutexPlayerList;
// std::mutex mutexClientSockets;


// std::vector<int> clientSockets;
// std::vector<Room> Rooms;
// std::set<std::string> playerList;


// std::unordered_map<int, clientInfo> clientInfoMap;
// std::unordered_map<std::string, int> nicknameToSocket;
// std::unordered_map<int, std::vector<int>> lobbyInfoMap;

int serverSocket;
const int PORT = 8080;



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
void shutdownJson(int signal){
    clearJsonFIle("serverJSONs/rooms.json");
    close(serverSocket);
    exit(0);
}


int main() {
    clearJsonFIle("serverJSONs/rooms.json");
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    std::signal(SIGINT, shutdownJson);
    if (serverSocket == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        return 1;
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


    //Init both ClientHandler and RoomHandler
    ClientHandler clientHandler;
    RoomHandler roomHandler(clientHandler);
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
        clientInfoMap[clientSocket] = {};
        clientSockets.push_back(clientSocket);
        printAllClients();
        std::thread([&clientHandler, clientSocket, &roomHandler]() {
            clientHandler.handleClient(clientSocket, roomHandler);
        }).detach();
    }
    close(serverSocket);
    return 0;
}
