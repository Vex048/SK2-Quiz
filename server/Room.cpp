#include "Room.h"  
#include <algorithm> 
#include <iostream>

// Room::Room(std::string roomName) {
//     name = roomName;
//     status = "waiting";
//     maxPlayers = 5;
//     currentQuestionIndex = -1;
// }

// Setter for status
void Room::setStatus(std::string newStatus) {
    status = newStatus;
}

// Getiing new Game master 
std::string Room::getNewGameMaster(){
    int numberOfPlayers = players.size();
    if (numberOfPlayers > 0){
        return players.front();
    } 
    return "No players in room";
}

// Setter for current question index
void Room::setIndex(int index){
    currentQuestionIndex=index;
}
// Getter for current question index
// If next question will be bigger than maxquestions its return 0 that means the game is fninished
int Room::getIndex(){
    if (currentQuestionIndex+1>maxQuestions){
        return 0;
    }
    return currentQuestionIndex;
}

// Below are some setters and getters 

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


// Adding player to the room, based on his nick and assiging 0 playerPoints to him 
void Room::addPlayer(int playerSocket,std::unordered_map<int, clientInfo> &clientInfoMap) {
    if (players.size() < maxPlayers) {
        std::string playerNick = clientInfoMap[playerSocket].nick;
        players.push_back(playerNick);
        playersPoints[playerSocket] = 0;
    } else {
        std::cerr << "Room " << name << " is full. Cannot add player " << playerSocket << std::endl;
    }
}


// Priting room informations, usefull for debugging
void Room::printRoomInfo(){
    // std::cout << Room::getRoomName() << std::endl;
    // std::cout << Room::getGameMaster() << std::endl;
    std::cout << "Room Name: " << name << std::endl;
    std::cout << "Game Master: " << gameMaster << std::endl;
}


// Removing player from room 
void Room::removePlayer(int playerSocket,std::unordered_map<int, clientInfo> clientInfoMap) {
    std::string playerNick = clientInfoMap[playerSocket].nick;
    players.erase(std::remove(players.begin(), players.end(), playerNick), players.end());
    playersPoints.erase(playerSocket);

    timestamp_playerleftroom = std::chrono::system_clock::now();
}

//Again some setters and getters

int Room::getNumberOfPlayers(){
    return players.size();
}


// timestamp_playerleftroom variable is used for removing a room if it is empty for more than a minute 
std::chrono::time_point<std::chrono::system_clock> Room::getTimeStampPlayerLeftRoom(){
    return timestamp_playerleftroom;
}

// timestamp_questions is used for an answer time while playing game, in our case its 15 seconds
std::chrono::time_point<std::chrono::system_clock> Room::getTimeStampPlayerQuestionUpdate(){
    return timestamp_questions;
}
void Room::setTimeStampQuestionUpdate(std::chrono::time_point<std::chrono::system_clock> timestamp){
    timestamp_questions = timestamp;
}


// Get all points from room, returning a json 
json Room::getAllPoints(std::unordered_map<int, clientInfo> clientInfoMap){
    json data;
    for(auto& element : playersPoints)
    {
        std::string nick = clientInfoMap[element.first].nick;
        data[nick] = element.second;
    }
    return data;
}


// Updateing player points after a guess
void Room::updatePlayersPoints(int playerSocket, std::string answer, std::unordered_map<int, clientInfo> clientInfoMap) {
    if(answer == curQuestion.correctAnswer){
        playersPoints[playerSocket] += 1;
    }
    curQuestion.numOfAnswers += 1;
}


// Setiing player points to 0, used when the game master dont want to use points from previous round
void Room::setZeroPlayerPoints(){
    for (auto & element : playersPoints){
    element.second = 0;
    }
}
// Setting new question and removing possibility for the same question again in a round 
void Room::setCurrentQuestion(int questionId, std::string questionText,std::vector<std::string>Options, 
                                std::string correctAnswer){
    curQuestion.questionId = questionId;
    curQuestion.questionText = questionText;
    curQuestion.correctAnswer = correctAnswer;
    curQuestion.options = Options;
    curQuestion.numOfAnswers = 0;
    removeQuestionIndex(questionId-1); // indices start from 0, QuestionId starts from 1
}

// Reseting indicies when the game ends
void Room::resetQuestionIndices(int categoryQuestionSize){
    questionIndices.clear();
    for(int i=0;i<categoryQuestionSize;i++){
        questionIndices.push_back(i);
    }
}
// Removing an questionId index, used for not having a repetitions in questions
void Room::removeQuestionIndex(int index){
    questionIndices.erase(std::remove(questionIndices.begin(), questionIndices.end(),index),questionIndices.end());
}
//Setter and getter for game Status
std::string Room::getStatus(){
    return status;
}
std::string Room::getCategory(){
    return category;
}

// Sending an data to all clients in current room 
void Room::sendToClientsInRoom(std::string data1,std::unordered_map<std::string, int> nicknameToSocket){
    for (const auto& player : players){
        int playerSocket = nicknameToSocket[player];  
        send(playerSocket, data1.c_str(), data1.size(), 0);   
    }
}

// Constant method that return jsons with room information
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



