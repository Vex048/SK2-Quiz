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
std::string Room::getNewGameMaster(){
    int numberOfPlayers = players.size();
    if (numberOfPlayers > 0){
        return players.front();
    } 
    return "No players in room";
}

void Room::setIndex(int index){
    currentQuestionIndex=index;
}
int Room::getIndex(){
    if (currentQuestionIndex+1>maxQuestions){
        return 0;
    }
    return currentQuestionIndex;
}

void Room::setCategory(std::string newCategory) {
    category = newCategory;
}
std::string Room::getRoomName(){
    return name;
}
std::string Room::getGameMaster(){
    return gameMaster;
}
void Room::setGameMaster(std::string player){
    gameMaster = player;
}
int Room::getMaxQustions(){
    return maxQuestions;
}
void Room::setMaxQuestions(int maxQ){
    maxQuestions = maxQ;
}


void Room::addPlayer(int playerSocket,std::unordered_map<int, clientInfo> &clientInfoMap) {
    if (players.size() < maxPlayers) {
        std::string playerNick = clientInfoMap[playerSocket].nick;
        players.push_back(playerNick);
        playersPoints[playerSocket] = 0;
    } else {
        std::cerr << "Room " << name << " is full. Cannot add player " << playerSocket << std::endl;
    }
}

void Room::printRoomInfo(){
    // std::cout << Room::getRoomName() << std::endl;
    // std::cout << Room::getGameMaster() << std::endl;
    std::cout << "Room Name: " << name << std::endl;
    std::cout << "Game Master: " << gameMaster << std::endl;
}

void Room::removePlayer(int playerSocket,std::unordered_map<int, clientInfo> clientInfoMap) {
    std::string playerNick = clientInfoMap[playerSocket].nick;
    players.erase(std::remove(players.begin(), players.end(), playerNick), players.end());
    playersPoints.erase(playerSocket);

    timestamp_playerleftroom = std::chrono::system_clock::now();
}
int Room::getNumberOfPlayers(){
    return players.size();
}

std::chrono::time_point<std::chrono::system_clock> Room::getTimeStampPlayerLeftRoom(){
    return timestamp_playerleftroom;
}
std::chrono::time_point<std::chrono::system_clock> Room::getTimeStampPlayerQuestionUpdate(){
    return timestamp_questions;
}
void Room::setTimeStampQuestionUpdate(std::chrono::time_point<std::chrono::system_clock> timestamp){
    timestamp_questions = timestamp;
}

json Room::getAllPoints(std::unordered_map<int, clientInfo> clientInfoMap){
    json data;
    for(auto& element : playersPoints)
    {
        std::string nick = clientInfoMap[element.first].nick;
        data[nick] = element.second;
    }
    return data;
}

void Room::updatePlayersPoints(int playerSocket, std::string answer, std::unordered_map<int, clientInfo> clientInfoMap) {
    if(answer == curQuestion.correctAnswer){
        playersPoints[playerSocket] += 1;
    }
    curQuestion.numOfAnswers += 1;
}

void Room::setZeroPlayerPoints(){
    for (auto & element : playersPoints){
    element.second = 0;
    }
}
void Room::setCurrentQuestion(int questionId, std::string questionText,std::vector<std::string>Options, 
                                std::string correctAnswer){
    curQuestion.questionId = questionId;
    curQuestion.questionText = questionText;
    curQuestion.correctAnswer = correctAnswer;
    curQuestion.options = Options;
    curQuestion.numOfAnswers = 0;
    removeQuestionIndex(questionId-1); // indices start from 0, QuestionId starts from 1
}


void Room::resetQuestionIndices(int categoryQuestionSize){
    questionIndices.clear();
    for(int i=0;i<categoryQuestionSize;i++){
        questionIndices.push_back(i);
    }
}
void Room::removeQuestionIndex(int index){
    questionIndices.erase(std::remove(questionIndices.begin(), questionIndices.end(),index),questionIndices.end());
}

std::string Room::getStatus(){
    return status;
}
std::string Room::getCategory(){
    return category;
}

void Room::sendToClientsInRoom(std::string data1,std::unordered_map<std::string, int> nicknameToSocket){
    for (const auto& player : players){
        int playerSocket = nicknameToSocket[player];  
        send(playerSocket, data1.c_str(), data1.size(), 0);   
    }
}


json Room::toJSON() const {

    json questionInfo = {
                {"questionId", curQuestion.questionId},
                {"questionText", curQuestion.questionText},
                {"options", curQuestion.options}
            };

    json roomInfo = {
                {"name", name},
                {"status", status},
                {"players", players},
                {"category", category},
                {"currentQuestionIndex", currentQuestionIndex},
                {"maxPlayers", maxPlayers},
                {"gameMaster",gameMaster},
                {"questionInfo", questionInfo}
                
            };
    return roomInfo;
}



