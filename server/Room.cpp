#include "Room.h"  
#include <algorithm> 
#include <iostream>

// Room::Room(std::string roomName) {
//     name = roomName;
//     status = "waiting";
//     maxPlayers = 5;
//     currentQuestionIndex = -1;
// }


void Room::setStatus(std::string newStatus) {
    status = newStatus;
}


void Room::setCategory(std::string newCategory) {
    category = newCategory;
}
std::string Room::getRoomName(){
    return name;
}


void Room::addPlayer(int playerSocket,std::unordered_map<int, clientInfo> &clientInfoMap) {
    if (players.size() < maxPlayers) {
        players.push_back(clientInfoMap[playerSocket].nick);
    } else {
        std::cerr << "Room " << name << " is full. Cannot add player " << playerSocket << std::endl;
    }
}

void Room::removePlayer(int playerSocket,std::unordered_map<int, clientInfo> clientInfoMap) {
    players.erase(std::remove(players.begin(), players.end(), clientInfoMap[playerSocket].nick), players.end());
}

json Room::toJSON() const {
    json roomInfo = {
                {"name", name},
                {"status", status},
                {"players", players},
                {"category", category},
                {"currentQuestionIndex", currentQuestionIndex},
                {"maxPlayers", maxPlayers}
            };
    return roomInfo;
}



