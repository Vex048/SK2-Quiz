#pragma once
#include <mutex>
#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include "Room.h"
#include <condition_variable>


// Global variables that are used by all classes 

extern std::unordered_map<std::string, std::condition_variable> roomCVMap; // Unordered map that binds room_name with condition variable
extern std::unordered_map<std::string, bool> roomFlagsEvents; // Unordered map that binds room_name with boolean that are used along condition variables
extern std::mutex roomCVMutex; // Mutex for condition variables


// Mutexes that are used to protect structures from multiple acces in the same time by threads 
extern std::mutex mutexRooms;
extern std::mutex mutexClientInfoMap;
extern std::mutex mutexPlayerList;
extern std::mutex mutexClientSockets;
extern std::mutex mutexLobbyClients;
extern std::mutex mutexRoomClients;

// Unordered map that binds room_name with client sockets in the room, helpful to limit sending data by server
extern std::unordered_map<std::string, std::set<int>> roomClients;  
// Set of client sockets that is also helpful with limiting sending data to clients by server 
extern std::set<int> lobbyClients;

// Vector with all client sockets
extern std::vector<int> clientSockets;
// Vector that contain all Room objects
extern std::vector<Room> Rooms;

//Set of a player-nicknames, used in checking for already existing nickname
extern std::set<std::string> playerList;

// Unordered map that bind client socket with a struct that contain nickname 
extern std::unordered_map<int, clientInfo> clientInfoMap;
//  Unordered map that bind nick to socket
extern std::unordered_map<std::string, int> nicknameToSocket;
