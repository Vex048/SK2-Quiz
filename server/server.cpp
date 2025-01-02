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
#include <fstream>
#include <csignal>
#include <set>
#include <mutex>
#include <time.h>
#include <chrono>
#include <optional>
using json = nlohmann::json;

std::mutex mutexRooms;
std::mutex mutexClientInfoMap;
std::mutex mutexPlayerList;
std::mutex mutexClientSockets;

const int PORT = 8080;
std::vector<int> clientSockets;
std::vector<Room> Rooms;
std::set<std::string> playerList;
int serverSocket;

std::unordered_map<int, clientInfo> clientInfoMap;
std::unordered_map<int, std::vector<int>> lobbyInfoMap;

void sendToAllClients(std::string message){
    mutexClientInfoMap.lock();
    for(auto& client: clientInfoMap){
        message= message + "\n";
        send(client.first, message.c_str(), message.size(), 0);       
    }
    mutexClientInfoMap.unlock();
}

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


// std::vector<std::string> splitMessage(const std::string& message, char delimiter) {
//     std::vector<std::string> parts;
//     std::istringstream stream(message);
//     std::string part;

//     while (std::getline(stream, part, delimiter)) {
//         parts.push_back(part);
//     }

//     return parts;
// }

void disconnectClient(int clientFd){
    mutexClientInfoMap.lock();
    mutexPlayerList.lock();
    close(clientFd);
    playerList.erase(clientInfoMap[clientFd].nick);
    clientInfoMap.erase(clientFd);
    // check if client is in any lobby and remove it
    for(auto &lobby: lobbyInfoMap){
        std::vector<int> &clients = lobby.second;
        auto it = std::find(clients.begin(),clients.end(),clientFd);
        if (it!=clients.end()){
            clients.erase(it);
        }
    }
    mutexClientInfoMap.unlock();
    mutexPlayerList.unlock();

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

Room* getRoomFromFile(std::string room_name){
    for (Room& room : Rooms) {
        std::string name = room.getRoomName();
        if (name == room_name){
            return &room;
        }      
    }
    return nullptr;
}


void handleRoom(std::string room_name){
    while(true){ 
        sleep(1);
        mutexRooms.lock();
        Room* room = getRoomFromFile(room_name);
        if (room==nullptr){
            mutexRooms.unlock();
            break;
        }
        int lastNumberOfPlayers = room->getNumberOfPlayers();        
        std::cout << "Number of players in room: " << lastNumberOfPlayers << std::endl;
        if(lastNumberOfPlayers==0){
            //room.timestamp_playerleftroom;
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = now - room->getTimeStamp();
            std::cout << "Elapsed time room empty: " << elapsed_seconds.count() << "s\n";
        }
        

        mutexRooms.unlock();
    }
}

void handleRoomCreate(json data,int clientsocket){
    if (data["name"] != nullptr){
        mutexRooms.lock();
        std::string room_name = data["name"];
        std::cout << room_name << std::endl;
        Room newRoom(room_name);
        newRoom.addPlayer(clientsocket,clientInfoMap);
        newRoom.setGameMaster(clientInfoMap[clientsocket].nick);
        Rooms.push_back(newRoom);
        std::cout << "Here room should be stored in json" << std::endl;
        roomsToFile(Rooms);
        json response;
        response["status"] = "success";
        response["type"] = "room_create";
        response["room_name"] = newRoom.name;
        response["players"] = newRoom.players;
        response["gameMaster"] = newRoom.getGameMaster();
        std::string responseStr = response.dump();

        sendToAllClients(responseStr);
        mutexRooms.unlock();

        std::thread(handleRoom, newRoom.getRoomName()).detach();
        //roomThread;
    }
    else{
        std::cout << "Room name is equall to null" << std::endl;
    }
}
void handleNickname(json data,int clientsocket){
    
    std::string nick = data["nickname"];
    json response;
    response["type"] = "create_nickname";
    mutexPlayerList.lock();
    if (playerList.find(nick) != playerList.end()){
        response["status"] = "failure";
    }
    else{
        response["status"] = "succes";
        response["nickname"] = nick;
        playerList.insert(nick);
        mutexClientInfoMap.lock();
        clientInfoMap[clientsocket].nick = nick;
        mutexClientInfoMap.unlock();
    }
    mutexPlayerList.unlock();
    std::string responseStr = response.dump();
    responseStr = responseStr + "\n";
    std::cout << responseStr << std::endl;
    send(clientsocket, responseStr.c_str(), responseStr.size(), 0);
    
    
}

void sendToClientsRoomsInfo(int clientsocket){
    std::ifstream ifs("serverJSONs/rooms.json");
    json jf = json::parse(ifs);
    json response;
    response["type"] = "rooms_info";
    if (jf["rooms"].size() > 0){
        
        response["status"] = "succes";
        response["rooms"] = jf["rooms"];
    }
    else{
        response["status"] = "failure";
    }

    
    
    std::string responseStr = response.dump();
    responseStr = responseStr + "\n";
    std::cout << responseStr << std::endl;
    sendToAllClients(responseStr);
    //send(clientsocket, responseStr.c_str(), responseStr.size(), 0); 
}

void handlePlayer(json data,int clientsocket){
    std::string room_name = data["name"];
    mutexRooms.lock();
    for (Room& room : Rooms) {
        std::string name = room.getRoomName();
        if (name == room_name){
            
            mutexClientInfoMap.lock();
            room.addPlayer(clientsocket,clientInfoMap);
            std::cout << "Player joined a lobby,number of players: " << room.getNumberOfPlayers() << std::endl;
            if (room.getGameMaster() == ""){
                room.setGameMaster(clientInfoMap[clientsocket].nick);
            }
            mutexClientInfoMap.unlock();
            roomsToFile(Rooms);
            sendToClientsRoomsInfo(clientsocket);
            
        }   
    }
    mutexRooms.unlock();
    
}

void checkIfGameMaster(int clientsocket,Room& room){
    std::string player = room.getGameMaster();
    std::cout << "Current Game master: " << player << std::endl;
    mutexClientInfoMap.lock();
    if (player == clientInfoMap[clientsocket].nick){
        std::string newGameMaster = room.getNewGameMaster();
        std::cout << "New Game master: " << newGameMaster << std::endl;
        if (newGameMaster != "No players in room"){
            room.setGameMaster(newGameMaster);
        }
        else {
            room.setGameMaster("");
        }
        
    }
    mutexClientInfoMap.unlock();  
}
void RemovePlayerFromRoom(json data,int clientsocket){
    std::string room_name = data["name"];
    mutexRooms.lock();
    for (Room& room : Rooms) {
            std::string name = room.getRoomName();
            if (name == room_name){
                mutexClientInfoMap.lock();
                room.removePlayer(clientsocket,clientInfoMap);
                std::cout << "Player removed from a lobby,number of players: " << room.getNumberOfPlayers() << std::endl;
                mutexClientInfoMap.unlock();
                checkIfGameMaster(clientsocket,room);
                roomsToFile(Rooms);
                sendToClientsRoomsInfo(clientsocket);
            }
    
        }
    mutexRooms.unlock(); 
}

void StartGame(json data,int clientsocket){
    mutexRooms.lock();
    std::string room_name = data["name"];
    for (Room& room : Rooms) {
            std::string name = room.getRoomName();
            if (name == room_name){
                room.setStatus("Started");
                room.setCategory(data["category"]);
                roomsToFile(Rooms);
                sendToClientsRoomsInfo(clientsocket);
            }
            
        }
    mutexRooms.unlock(); 
}

void manageMessage(json data,int clientFd){

    if (data["type"] == "create_room"){
        handleRoomCreate(data,clientFd);
    }
    else if (data["type"] == "create_nickname"){
        handleNickname(data,clientFd);
    }
    else if (data["type"] == "rooms_info"){
        mutexRooms.lock();
        sendToClientsRoomsInfo(clientFd);
        mutexRooms.unlock();
    }
    else if (data["type"] == "player_join_room"){
        handlePlayer(data,clientFd);
    }
    else if (data["type"] == "player_exit_room"){
        RemovePlayerFromRoom(data,clientFd);
    }
    else if (data["type"] == "start_game"){
        StartGame(data,clientFd);
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
    buffer[n] = '\0';
    std::string data(buffer);

    std::cout << " Received1: " << buffer<< std::endl;
    //json data = json::parse (buffer);

    size_t pos;
    while ((pos = data.find("\n")) != std::string::npos) {
            std::string jsonStr = data.substr(0, pos);
            data.erase(0, pos + 1);

            try {
                json message = json::parse(jsonStr);
                manageMessage(message,clientFd); 
            } catch (const json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        }


    return n;
}



void handleClient(int clientSocket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int n = readMessage(clientSocket,buffer,sizeof(buffer)-1);
        if(n<=0) return;
        
        // std::string response = "QUESTION|What is 2+2?";
        // send(clientSocket, response.c_str(), response.size(), 0);
    }
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
        std::thread(handleClient, clientSocket).detach();
    }
    close(serverSocket);
    return 0;
}
