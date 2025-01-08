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
#include <random>
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
std::unordered_map<std::string, int> nicknameToSocket;
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


void disconnectClient(int clientFd){
    mutexClientInfoMap.lock();
    mutexPlayerList.lock();

    std::string nick = clientInfoMap[clientFd].nick;
    close(clientFd);
    playerList.erase(nick);
    nicknameToSocket.erase(nick);
    clientInfoMap.erase(clientFd);
    // check if client is in any lobby and remove it
    // for(auto &lobby: lobbyInfoMap){
    //     std::vector<int> &clients = lobby.second;
    //     auto it = std::find(clients.begin(),clients.end(),clientFd);
    //     if (it!=clients.end()){
    //         clients.erase(it);
    //     }
    // }
    mutexRooms.lock();
    for(Room& room: Rooms){ // doesnt work :((
        auto it = std::find(room.players.begin(),room.players.end(),nick);
        std::cout << "Player removed from room: " << room.getRoomName() << std::endl;
        if(it!=room.players.end()){
            room.removePlayer(clientFd,clientInfoMap);
            std::cout << "Player removed from room: " << room.getRoomName() << ", number of players: " << room.getNumberOfPlayers() << std::endl;
        }
    }
    roomsToFile(Rooms);
    mutexRooms.unlock();
    mutexClientInfoMap.unlock();
    mutexPlayerList.unlock();
    

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
void removeRoom(std::string room_name){
    for (auto it = Rooms.begin();it!=Rooms.end();it++){
        if (it->getRoomName() ==room_name){
            Rooms.erase(it);
            roomsToFile(Rooms);
            // send info to clients so that clients don't have to refresh manually
            mutexRooms.unlock();
            break;
        }
    }
}



void GetAnswerFromClient(json data,int clientFd){
    if(data["selectedOption"] != nullptr){
        std::string answer = data["selectedOption"];
        std::string room_name = data["roomName"];
        mutexRooms.lock();
        Room *room = getRoomFromFile(room_name);
        if (room!=nullptr){
            room->updatePlayersPoints(clientFd,answer,clientInfoMap);
            std::cout << "Player " << clientInfoMap[clientFd].nick << " answered: " << answer << ", points: "<< room->playersPoints[clientFd] << std::endl;
            roomsToFile(Rooms);
        }
        mutexRooms.unlock();
    }
    std::cout << "Answer from client: "<<data<< std::endl;
    return;
}


void handleRoom(std::string room_name){
    std::ifstream questionsFile("serverJSONs/questions.json");
    json questionsJson;
    questionsFile >> questionsJson;
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
            std::chrono::duration<double> elapsed_seconds = now - room->getTimeStampPlayerLeftRoom();
            std::cout << "Elapsed time room empty: " << elapsed_seconds.count() << "s\n";
            if (elapsed_seconds.count() > 10){ 
                std::cout << "Room is empty for more than 5 minutes, deleting room" << std::endl;
                removeRoom(room_name);
                roomsToFile(Rooms); 
                sendToClientsRoomsInfo(0);
                mutexRooms.unlock();
                break;
            }
        }
        else{ 
            if (room->getStatus() == "Started"){
                std::string RoomCategory = room->getCategory();
                std::chrono::time_point<std::chrono::system_clock> now1 = std::chrono::system_clock::now();
                std::chrono::duration<double> elapsed_seconds2 = now1 - room->getTimeStampPlayerQuestionUpdate();
                std::cout << "Elapsed time for question: " << elapsed_seconds2.count() << "s\n";

                if (elapsed_seconds2.count() > 15){ 
                    std::cout << "15 second for question has finished" << std::endl;
                    
                    json categoryQuestions = questionsJson["categories"][RoomCategory];

                    std::random_device dev;
                    std::mt19937 rng(dev());
                    std::uniform_int_distribution<std::mt19937::result_type> dist(0,categoryQuestions.size()-1);
                    int randomIndex = dist(rng);
                    int index = room->getIndex();
                    if (index == 0){
                        std::cout << "Game is finished" << std::endl;
                        json response;                    
                        response["type"] = "game_finished";
                        json scores = room->getAllPoints(clientInfoMap);
                        std::string scoresStr = scores.dump();
                        scoresStr = scoresStr + "\n";
                        std:: cout << scoresStr << std::endl;
                        response["scores"] = scores;
                        std::string responseStr = response.dump();
                        responseStr = responseStr + "\n";
                        room->sendToClientsInRoom(responseStr,nicknameToSocket);
                        room->setStatus("Waiting");
                        roomsToFile(Rooms);

                    }
                    else {                   
                    room->setIndex(index+1);
                    room->setCurrentQuestion(categoryQuestions[randomIndex]["questionNumber"],
                    categoryQuestions[randomIndex]["question"],categoryQuestions[randomIndex]["options"],
                    categoryQuestions[randomIndex]["correctAnswer"]);

                    room->setTimeStampQuestionUpdate(std::chrono::system_clock::now());                   
                    roomsToFile(Rooms); 
                    json response;
                    response["type"] = "new_question";
                    
                    json quest;
                    quest["questionText"] = categoryQuestions[randomIndex]["question"];
                    quest["options"] = categoryQuestions[randomIndex]["options"];
                    quest["questionId"] = categoryQuestions[randomIndex]["questionNumber"];
                    response["data"] = quest;
                    std::string responseStr = response.dump();
                    responseStr = responseStr + "\n";
                    room->sendToClientsInRoom(responseStr,nicknameToSocket);
                    //sendToClientsRoomsInfo(0);   
                    }                
                }
            }
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
        nicknameToSocket[nick] = clientsocket;
        mutexClientInfoMap.unlock();
    }
    mutexPlayerList.unlock();
    std::string responseStr = response.dump();
    responseStr = responseStr + "\n";
    std::cout << responseStr << std::endl;
    send(clientsocket, responseStr.c_str(), responseStr.size(), 0);
    
    
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

void toLowerCase(std::string &str){
    std::transform(str.begin(),str.end(),str.begin(),::tolower);
}

void StartGame(json data,int clientsocket){

    std::ifstream questionsFile("serverJSONs/questions.json");
    json questionsJson;
    questionsFile >> questionsJson;
    // print entire json
    std::cout << questionsJson["categories"] << std::endl;



    mutexRooms.lock();
    std::string room_name = data["name"];
    std::string category = data["category"];
    std::string savePoints = data["save_points"];
    toLowerCase(category); // in json categories are stored in lowercase
    std::cout << "Category: " << category << std::endl;


    if (questionsJson["categories"].contains(category)) {
        json categoryQuestions = questionsJson["categories"][category];

        for (Room& room : Rooms) {
                std::string name = room.getRoomName();
                if (name == room_name){
                    if (savePoints == "No"){
                        room.setZeroPlayerPoints();
                    }

                    room.setStatus("Started");
                    room.setCategory(category);
                    //  std::cout << categoryQuestions[0]["questionNumber"] <<", "<< categoryQuestions[0]["question"] << ", " << categoryQuestions[0]["options"] << categoryQuestions[0]["correctAnswer"] << std::endl;
                    std::cout << "Category questions: " << categoryQuestions << std::endl;
                    std::random_device dev;
                    std::mt19937 rng(dev());
                    std::uniform_int_distribution<std::mt19937::result_type> dist(0,categoryQuestions.size()-1);
                    int randomIndex = dist(rng);
                    room.setIndex(1);
                    room.setCurrentQuestion(categoryQuestions[randomIndex]["questionNumber"],categoryQuestions[randomIndex]["question"],categoryQuestions[randomIndex]["options"],categoryQuestions[randomIndex]["correctAnswer"]);
                    room.setTimeStampQuestionUpdate(std::chrono::system_clock::now());
                    roomsToFile(Rooms);
                    sendToClientsRoomsInfo(clientsocket);

                    mutexRooms.unlock(); // after room is found and started, unlock mutex and exit function
                    return;
                }
                
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
    else if (data["type"] == "answer"){
        GetAnswerFromClient(data,clientFd);
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
