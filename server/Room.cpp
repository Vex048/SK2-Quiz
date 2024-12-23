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


void Room::addPlayer(int playerSocket) {
    if (players.size() < maxPlayers) {
        players.push_back(playerSocket);
    } else {
        std::cerr << "Room " << name << " is full. Cannot add player " << playerSocket << std::endl;
    }
}

void Room::removePlayer(int playerSocket) {
    players.erase(std::remove(players.begin(), players.end(), playerSocket), players.end());
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



