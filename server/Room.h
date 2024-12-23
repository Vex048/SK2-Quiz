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
using json = nlohmann::json;

class Room {
    public:
        Room(std::string roomName){ 
            name = roomName; 
            status = "waiting";
            maxPlayers=5; 
        };
        std::string name;
        int maxPlayers;
        std::vector<int> players;
        std::string status;     
        std::string category;               
        //std::<vector><json> questions;        
        int currentQuestionIndex;      
        void setStatus(std::string status);
        void setCategory(std::string status);
        void addPlayer(int playerSocket);
        void removePlayer(int playerSocket);
        json toJSON() const;
};
