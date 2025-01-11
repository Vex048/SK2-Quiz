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
            //Setting default values of room 
            name = roomName; 
            status = "Waiting";
            maxPlayers=5;
            maxQuestions=3;
        };
        //Structure that represent a one question
        struct currentQuestion{
            int questionId;
            std::string questionText;
            std::string correctAnswer;
            std::vector<std::string> options;
            int numOfAnswers;
        };
        std::string name;
        int maxPlayers;
        std::vector<std::string> players;
        std::string status;     
        std::string category; 
        std::string gameMaster;
        int maxQuestions;
        std::vector <int> questionIndices; // indices of questions from chosen category to eliminate occurence of the same question
        /* [0,1,2,3,4,5,6,7,8,9,10] get size of given category from json during start game->create vector of this size->
        ->draw question index based on the remaininng indices->proceed with the rest of already implemented code
        */
        struct currentQuestion curQuestion;
        int currentQuestionIndex;

        //Timestamps that are controlling seconds in both game and room 
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
        int getMaxQustions();
        void setMaxQuestions(int maxQ);
        void resetQuestionIndices(int categoryQuestionSize);
        void removeQuestionIndex(int index);

        void addPlayer(int playerSocket,std::unordered_map<int, clientInfo> &clientInfoMap);
        void removePlayer(int playerSocket,std::unordered_map<int, clientInfo> clientInfoMap);
        int getNumberOfPlayers();
        
        void sendToClientsInRoom(std::string data,std::unordered_map<std::string, int> nicknameToSocket);
        
        std::string getRoomName();
        void printRoomInfo();
        json toJSON() const;
};
