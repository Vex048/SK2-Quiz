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
#include <fstream>
#include <csignal>
#include <set>
#include <mutex>
#include <time.h>
#include <chrono>
#include <random>
#include <optional>
//#include "globals.h"
#pragma once
#include "RoomHandler.h"
using json = nlohmann::json;


class ClientHandler{
    public:
    
    void handleClient(int clientsocket,RoomHandler& roomHandler);
    int readMessage(int clientFd, char * buffer,int bufSize,RoomHandler& roomHandler);
    void manageMessage(json data,int clientFd,RoomHandler& roomHandler);

    void handleNickname(json data,int clientsocket);
    void GetAnswerFromClient(json data,int clientFd,RoomHandler& roomHandler);
    void disconnectClient(int clientFd,RoomHandler& roomHandler);

    void sendToAllClients(std::string response);
    void sendToClient(int clientsocket,std::string message);
    void sendToLobbyClientsRoomsInfo(std::string response);
    void sendToLobbyClientsRoomsInfo();
    void sendToRoomClientsRoomsInfo(std::string response,std::string room_name);
    void sendToRoomClientsRoomsInfo(std::string room_name);
    void sendToClientInfoRooms(int clientsocket);
    std::string getRoomsInfo();
};