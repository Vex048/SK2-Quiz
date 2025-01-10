#include "globals.h"
std::mutex mutexRooms;
std::mutex mutexClientInfoMap;
std::mutex mutexPlayerList;
std::mutex mutexClientSockets;

std::vector<int> clientSockets;
std::vector<Room> Rooms;

std::set<std::string> playerList;

std::unordered_map<int, clientInfo> clientInfoMap;
std::unordered_map<std::string, int> nicknameToSocket;
std::unordered_map<int, std::vector<int>> lobbyInfoMap;