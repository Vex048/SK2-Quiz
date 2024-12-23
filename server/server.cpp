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
#include "Room.h"

using json = nlohmann::json;

const int PORT = 8080;
std::vector<int> clientSockets;
std::vector<Room> Rooms;

struct clientInfo{
    std::string nick;
    // later on we can add more info about client such as score, current_quiz, etc.
};
std::unordered_map<int, clientInfo> clientInfoMap;
std::unordered_map<int, std::vector<int>> lobbyInfoMap;

void sendToAllClients(std::string message){
    for(auto& client: clientInfoMap){
        send(client.first, message.c_str(), message.size(), 0);       
    }
}

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

void destroyLobby(int LobbyId){
    lobbyInfoMap.erase(LobbyId);
    std::cout << "Lobby " << LobbyId << " destroyed" << std::endl;
}
void handleLobby(int LobbyId){
    while(true){

        
        std::vector<int> clients = lobbyInfoMap[LobbyId];
        std::cout<<"----------------------------------"<<std::endl;
        std::cout << "Lobby " << LobbyId << " has " << clients.size() << " clients" << std::endl;
        for(auto& clientfd: clients){
            std::cout << "Client's nickname: " << clientInfoMap[clientfd].nick << std::endl;
        }
        sleep(5);
        if(clients.size()==0){
            destroyLobby(LobbyId);
            return;
        }
    }
}
void createLobby(int LobbyId){
    lobbyInfoMap[LobbyId] = {};
    std::thread(handleLobby,LobbyId).detach();
}
void joinLobby(int LobbyId, int clientFd){

    // check if client is already in a lobby - client can't be in multiple lobbies at the same time
    for(auto& lobby: lobbyInfoMap){
        std::vector<int> &clients = lobby.second;
        auto it = std::find(clients.begin(),clients.end(),clientFd);
        if(it!=clients.end()){
            return; // client is already in a lobby, terminate function
        }
    }

    // check if lobby exists, if not create it
    if(lobbyInfoMap.find(LobbyId)==lobbyInfoMap.end()){
        createLobby(LobbyId);
    }
    lobbyInfoMap[LobbyId].push_back(clientFd);
    std::cout << "Client " << clientInfoMap[clientFd].nick << " joined lobby " << LobbyId << std::endl;
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
    // check if client is in any lobby and remove it
    for(auto &lobby: lobbyInfoMap){
        std::vector<int> &clients = lobby.second;
        auto it = std::find(clients.begin(),clients.end(),clientFd);
        if (it!=clients.end()){
            clients.erase(it);
        }
    }

}

void writeToFile(json data){
    std::ofstream o("serverJSONs/rooms.json");
    if (!o) {
        std::cerr << "Error: Unable to open file for writing.\n";
        return;
    }
    o << std::setw(4) << data << std::endl; // Pretty print with indentation
    o.close();
}


void roomsToFile(std::vector<Room>& rooms){
    json data;
    data["rooms"] = json::array();
     for (const Room& room : rooms) {
        json roomJson = room.toJSON();
        data["rooms"].push_back(roomJson);
    }
    writeToFile(data);
}

void handleRoomCreate(json data,int clientsocket){
    if (data["name"] != nullptr){
        std::string room_name = data["name"];
        std::cout << room_name << std::endl;
        Room newRoom(room_name);
        newRoom.addPlayer(clientsocket);
        Rooms.push_back(newRoom);
        std::cout << "Here room should be stored in json" << std::endl;
        roomsToFile(Rooms);
        json response;
        response["status"] = "success";
        response["type"] = "room_create";
        response["room_name"] = room_name;
        response["players"] = 1;
        std::string responseStr = response.dump();
        sendToAllClients(responseStr);
    }
    else{
        std::cout << "Room name is equall to null" << std::endl;
    }
}


int readMessage(int clientFd, char * buffer,int bufSize){   
    int n = recv(clientFd,buffer,bufSize,0);
    if (n<=0){
        if(n<0) error(0,errno,"Error on read from client %d",clientFd);

        disconnectClient(clientFd);
        std::cout << "Client disconnected gracefully\n";
        return -1;
    }

    std::cout << " Received1: " << buffer<< std::endl;
    json data = json::parse (buffer);
    handleRoomCreate(data,clientFd);
    std::cout << " Received2: " << data<< std::endl;
    return n;
}



void handleClient(int clientSocket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int n = readMessage(clientSocket,buffer,sizeof(buffer));
        if(n<=0) return;
        
        // std::string response = "QUESTION|What is 2+2?";
        // send(clientSocket, response.c_str(), response.size(), 0);
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
        clientInfoMap[clientSocket] = {};
        clientSockets.push_back(clientSocket);
        std::thread(handleClient, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}
