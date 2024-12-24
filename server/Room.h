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
#include "json.hpp"
#include <sstream>
#include <error.h>
#include <errno.h>
#include "clientInfo.h"
using json = nlohmann::json;

class Room {
    public:
        Room(std::string roomName){ 
            name = roomName; 
            status = "Waiting";
            maxPlayers=5; 
        };
        std::string name;
        int maxPlayers;
        std::vector<std::string> players;
        std::string status;     
        std::string category;               
        //std::<vector><json> questions;        
        int currentQuestionIndex;      
        void setStatus(std::string status);
        void setCategory(std::string status);
        void addPlayer(int playerSocket,std::unordered_map<int, clientInfo> &clientInfoMap);
        std::string getRoomName();
        //void removePlayer(int playerSocket,std::unordered_map<int, clientInfo> clientInfoMap);
        json toJSON() const;
};
