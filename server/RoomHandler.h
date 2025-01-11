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
#include "globals.h"
#pragma once

using json = nlohmann::json;
class ClientHandler;
class RoomHandler{
    private:
    ClientHandler& clientHandler;
    public:
    RoomHandler(ClientHandler& handler) : clientHandler(handler) {};
    void handleRoom(std::string room_name);
    void handleRoomCreate(json data,int clientsocket);
    void removeRoom(std::string room_name);
    Room* getRoomFromFile(std::string room_name);
    void writeToFile(json data);
    void roomsToFile(std::vector<Room>& rooms);
    void RemovePlayerFromRoom(json data,int clientsocket);
    void checkIfGameMaster(int clientsocket,Room& room);
    void handlePlayer(json data,int clientsocket);
    void StartGame(json data);
    void toLowerCase(std::string &str);
};