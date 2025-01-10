#pragma once
#include <mutex>
#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include "Room.h"
extern std::mutex mutexRooms;
extern std::mutex mutexClientInfoMap;
extern std::mutex mutexPlayerList;
extern std::mutex mutexClientSockets;

extern std::vector<int> clientSockets;
extern std::vector<Room> Rooms;

extern std::set<std::string> playerList;

extern std::unordered_map<int, clientInfo> clientInfoMap;
extern std::unordered_map<std::string, int> nicknameToSocket;
extern std::unordered_map<int, std::vector<int>> lobbyInfoMap;