#include "ClientHandler.h"

int ClientHandler::readMessage(int clientFd, char * buffer,int bufSize,RoomHandler& roomHandler){
    int n = recv(clientFd,buffer,bufSize,0);
    if (n<=0){
        if(n<0) error(0,errno,"Error on read from client %d",clientFd);

        disconnectClient(clientFd,roomHandler);
        std::cout << "Client disconnected gracefully\n";
        return -1;
    }
    buffer[n] = '\0';
    std::string data(buffer);
    std::cout << " Received1: " << buffer<< std::endl;
    size_t pos;
    while ((pos = data.find("\n")) != std::string::npos) {
            std::string jsonStr = data.substr(0, pos);
            data.erase(0, pos + 1);

            try {
                json message = json::parse(jsonStr);
                manageMessage(message,clientFd,roomHandler); 
            } catch (const json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        }

    return n;
}

void ClientHandler::sendToAllClients(std::string message){
    mutexClientInfoMap.lock();
    for(auto& client: clientInfoMap){
        message= message + "\n";
        send(client.first, message.c_str(), message.size(), 0);       
    }
    mutexClientInfoMap.unlock();
}

void ClientHandler::handleClient(int clientSocket,RoomHandler& roomHandler){
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int n = readMessage(clientSocket,buffer,sizeof(buffer)-1,roomHandler);
        if(n<=0) return;
        
    }
}

void ClientHandler::manageMessage(json data,int clientFd,RoomHandler& roomHandler){
    if (data["type"] == "create_room"){
        //handleRoomCreate(data,clientFd);
        roomHandler.handleRoomCreate(data,clientFd);
    }
    else if (data["type"] == "create_nickname"){
        handleNickname(data,clientFd);
    }
    else if (data["type"] == "rooms_info"){
        mutexRooms.lock();
        sendToClientsRoomsInfo();
        mutexRooms.unlock();
    }
    else if (data["type"] == "player_join_room"){
        roomHandler.handlePlayer(data,clientFd);
    }
    else if (data["type"] == "player_exit_room"){
        //RemovePlayerFromRoom(data,clientFd);
        roomHandler.RemovePlayerFromRoom(data,clientFd);
    }
    else if (data["type"] == "start_game"){
        roomHandler.StartGame(data);
    }
    else if (data["type"] == "answer"){
        GetAnswerFromClient(data,clientFd,roomHandler);
    }
}






void ClientHandler::handleNickname(json data,int clientsocket){
    
    std::string nick = data["nickname"];
    json response;
    response["type"] = "create_nickname";
    mutexPlayerList.lock();
    if (playerList.find(nick) != playerList.end()){
        response["status"] = "failure";
    }
    else{
        response["status"] = "succes";
        response["nickname"] = nick;
        playerList.insert(nick);
        mutexClientInfoMap.lock();
        clientInfoMap[clientsocket].nick = nick;
        nicknameToSocket[nick] = clientsocket;
        mutexClientInfoMap.unlock();
    }
    mutexPlayerList.unlock();
    std::string responseStr = response.dump();
    responseStr = responseStr + "\n";
    std::cout << responseStr << std::endl;
    send(clientsocket, responseStr.c_str(), responseStr.size(), 0);   
}

void ClientHandler::GetAnswerFromClient(json data,int clientFd,RoomHandler& roomHandler){
    if(data["selectedOption"] != nullptr){
        std::string answer = data["selectedOption"];
        std::string room_name = data["roomName"];
        mutexRooms.lock();
        Room *room = roomHandler.getRoomFromFile(room_name);
        if (room!=nullptr){
            room->updatePlayersPoints(clientFd,answer);
            std::cout << "Player " << clientInfoMap[clientFd].nick << " answered: " << answer << ", points: "<< room->playersPoints[clientFd] << std::endl;
            roomHandler.roomsToFile(Rooms);
        }
        mutexRooms.unlock();
    }
    std::cout << "Answer from client: "<<data<< std::endl;
    return;
}



void ClientHandler::disconnectClient(int clientFd,RoomHandler& roomHandler){
    mutexClientInfoMap.lock();
    mutexPlayerList.lock();

    std::string nick = clientInfoMap[clientFd].nick;
    close(clientFd);
    mutexRooms.lock();
    for(Room& room: Rooms){ 
        auto it = std::find(room.players.begin(),room.players.end(),nick);
        std::cout << "Player removed from room: " << room.getRoomName() << std::endl;
        if(it!=room.players.end()){
            room.removePlayer(clientFd,clientInfoMap);
            std::cout << "Player removed from room: " << room.getRoomName() << ", number of players: " << room.getNumberOfPlayers() << std::endl;
        }
    }
    roomHandler.roomsToFile(Rooms);
    mutexRooms.unlock();

    playerList.erase(nick);
    nicknameToSocket.erase(nick);
    clientInfoMap.erase(clientFd);

    mutexClientInfoMap.unlock();
    mutexPlayerList.unlock();
    

}


void ClientHandler::sendToClientsRoomsInfo(){
    std::ifstream ifs("serverJSONs/rooms.json");
    json jf = json::parse(ifs);
    json response;
    response["type"] = "rooms_info";
    if (jf["rooms"].size() > 0){
        
        response["status"] = "succes";
        response["rooms"] = jf["rooms"];
    }
    else{
        response["status"] = "failure";
    }

    std::string responseStr = response.dump();
    responseStr = responseStr + "\n";
    std::cout << responseStr << std::endl;
    sendToAllClients(responseStr);
    //send(clientsocket, responseStr.c_str(), responseStr.size(), 0); 
}


