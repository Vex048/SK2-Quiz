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

class Room {
    public:
        Room(std::string roomName){ 
            name = roomName;      
        };
        std::string name;
        bool gameStarted;
};
