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
            
        };

        struct currentQuestion{
            int questionNumber;
            std::string questionText;
            std::string correctAnswer;
            std::vector<std::string> options;
            std::unordered_map<int,std::string> playersAnswers; // <playerNick, answer>
        };

        std::string name;
        int maxPlayers;
        std::vector<std::string> players;
        std::string status;     
        std::string category; 
        std::string gameMaster;
        struct currentQuestion curQuestion;
    

        std::chrono::time_point<std::chrono::system_clock> timestamp_playerleftroom;
        std::chrono::time_point<std::chrono::system_clock> getTimeStamp();
        //std::<vector><json> questions;        
        int currentQuestionIndex;
        void setGameMaster(std::string player);
        void printRoomInfo();
        std::string getNewGameMaster();
        std::string getGameMaster();   
        void setStatus(std::string status);
        void setCategory(std::string category);
        void addPlayer(int playerSocket,std::unordered_map<int, clientInfo> &clientInfoMap);
        int getNumberOfPlayers();
        std::string getRoomName();
        void removePlayer(int playerSocket,std::unordered_map<int, clientInfo> clientInfoMap);
        void setCurrentQuestion(int questionNumber, std::string questionText,std::vector<std::string>Options, 
                                std::string correctAnswer, std::unordered_map<int,std::string> playersAnswers);
        json toJSON() const;
};
