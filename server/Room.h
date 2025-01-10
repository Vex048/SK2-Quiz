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
#include <chrono>
#include "clientInfo.h"
using json = nlohmann::json;

class Room {
    public:
        Room(std::string roomName){ 
            name = roomName; 
            status = "Waiting";
            maxPlayers=5;
            maxQuestions=3;
        };

        struct currentQuestion{
            int questionId;
            std::string questionText;
            std::string correctAnswer;
            std::vector<std::string> options;
        };

        std::string name;
        int maxPlayers;
        std::vector<std::string> players;
        std::string status;     
        std::string category; 
        std::string gameMaster;
        int maxQuestions;
        struct currentQuestion curQuestion;
        int currentQuestionIndex;

        std::chrono::time_point<std::chrono::system_clock> timestamp_playerleftroom;
        std::chrono::time_point<std::chrono::system_clock> timestamp_questions;

        std::unordered_map<int,int> playersPoints; // key: players socket, value: number of points
        
        void updatePlayersPoints(int playerSocket, std::string answer, std::unordered_map<int, clientInfo> clientInfoMap);     
        json getAllPoints(std::unordered_map<int, clientInfo> clientInfoMap);
        void setZeroPlayerPoints();

        std::chrono::time_point<std::chrono::system_clock> getTimeStampPlayerLeftRoom();
        std::chrono::time_point<std::chrono::system_clock> getTimeStampPlayerQuestionUpdate();  

    
        void setGameMaster(std::string player);
        std::string getNewGameMaster();
        std::string getGameMaster(); 

        void setStatus(std::string status);
        std::string getStatus();

        void setIndex(int index);
        int getIndex();

        std::string getCategory();
        void setCategory(std::string category);
        void setCurrentQuestion(int questionId, std::string questionText,std::vector<std::string>Options, 
                                std::string correctAnswer);
        void setTimeStampQuestionUpdate(std::chrono::time_point<std::chrono::system_clock> timestamp);


        void addPlayer(int playerSocket,std::unordered_map<int, clientInfo> &clientInfoMap);
        void removePlayer(int playerSocket,std::unordered_map<int, clientInfo> clientInfoMap);
        int getNumberOfPlayers();
        
        void sendToClientsInRoom(std::string data,std::unordered_map<std::string, int> nicknameToSocket);
        
        std::string getRoomName();
        void printRoomInfo();
        json toJSON() const;
};
