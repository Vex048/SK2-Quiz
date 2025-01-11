#include "RoomHandler.h"
#include "ClientHandler.h"

void RoomHandler::handleRoom(std::string room_name){
    std::ifstream questionsFile("serverJSONs/questions.json");
    json questionsJson;
    questionsFile >> questionsJson;
    while(true){ 
        sleep(1);
        mutexRooms.lock();
        Room* room = getRoomFromFile(room_name);
        if (room==nullptr){
            mutexRooms.unlock();
            break;
        }
        int lastNumberOfPlayers = room->getNumberOfPlayers();        
        std::cout << "Number of players in room: " << lastNumberOfPlayers << std::endl;
        if(lastNumberOfPlayers==0){
            //room.timestamp_playerleftroom;
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = now - room->getTimeStampPlayerLeftRoom();
            std::cout << "Elapsed time room empty: " << elapsed_seconds.count() << "s\n";
            if (elapsed_seconds.count() > 10){ 
                std::cout << "Room is empty for more than 5 minutes, deleting room" << std::endl;
                removeRoom(room_name);
                roomsToFile(Rooms); 
                clientHandler.sendToClientsRoomsInfo(0);
                // TUUTAJ ZROBIĆ COŚ Z WYSYŁNIAME DO KLIENTÓW
                mutexRooms.unlock();
                break;
            }
        }
        else{ 
            if (room->getStatus() == "Started"){
                std::string RoomCategory = room->getCategory();
                std::chrono::time_point<std::chrono::system_clock> now1 = std::chrono::system_clock::now();
                std::chrono::duration<double> elapsed_seconds2 = now1 - room->getTimeStampPlayerQuestionUpdate();
                std::cout << "Elapsed time for question: " << elapsed_seconds2.count() << "s\n";

                if (elapsed_seconds2.count() > 15 || room->curQuestion.numOfAnswers == room->getNumberOfPlayers()){ 
                    std::cout << "15 second for question has finished" << std::endl;
                    
                    json categoryQuestions = questionsJson["categories"][RoomCategory];

                    std::random_device dev;
                    std::mt19937 rng(dev());
                    int index = room->getIndex();
                    if (index == 0){
                        std::cout << "Game is finished" << std::endl;
                        json response;                    
                        response["type"] = "game_finished";
                        json scores = room->getAllPoints(clientInfoMap);
                        std::string scoresStr = scores.dump();
                        scoresStr = scoresStr + "\n";
                        std:: cout << scoresStr << std::endl;
                        response["scores"] = scores;
                        std::string responseStr = response.dump();
                        responseStr = responseStr + "\n";
                        room->sendToClientsInRoom(responseStr,nicknameToSocket);
                        room->setStatus("Waiting");
                        roomsToFile(Rooms);

                    }
                    else {               
                    std::uniform_int_distribution<std::mt19937::result_type> dist(0,room->questionIndices.size()-1);
                    int randomIndex = dist(rng);
                    randomIndex = room->questionIndices[randomIndex];
                    room->setIndex(index+1);
                    room->setCurrentQuestion(categoryQuestions[randomIndex]["questionId"],
                        categoryQuestions[randomIndex]["questionText"],categoryQuestions[randomIndex]["options"],
                        categoryQuestions[randomIndex]["correctAnswer"]);

                    room->setTimeStampQuestionUpdate(std::chrono::system_clock::now());                   
                    roomsToFile(Rooms); 
                    json response;
                    response["type"] = "new_question";
                    
                    json quest;
                    quest["questionText"] = categoryQuestions[randomIndex]["questionText"];
                    quest["options"] = categoryQuestions[randomIndex]["options"];
                    quest["questionId"] = categoryQuestions[randomIndex]["questionId"];
                    response["data"] = quest;
                    std::string responseStr = response.dump();
                    responseStr = responseStr + "\n";
                    room->sendToClientsInRoom(responseStr,nicknameToSocket);
                    //sendToClientsRoomsInfo(0);   
                    }                
                }
            }
        }
        
        mutexRooms.unlock();
    }
}

void RoomHandler::removeRoom(std::string room_name){
    for (auto it = Rooms.begin();it!=Rooms.end();it++){
        if (it->getRoomName() ==room_name){
            Rooms.erase(it);
            roomsToFile(Rooms);
            // send info to clients so that clients don't have to refresh manually
            mutexRooms.unlock();
            break;
        }
    }
}

Room* RoomHandler::getRoomFromFile(std::string room_name){
    for (Room& room : Rooms) {
        std::string name = room.getRoomName();
        if (name == room_name){
            return &room;
        }      
    }
    return nullptr;
}

void RoomHandler::writeToFile(json data){
    std::ofstream o("serverJSONs/rooms.json");
    if (!o) {
        std::cerr << "Error: Unable to open file for writing.\n";
        return;
    }
    o << std::setw(4) << data << std::endl; // Pretty print with indentation
    o.close();
}

void RoomHandler::roomsToFile(std::vector<Room>& rooms){
    json data;
    data["rooms"] = json::array();
     for (const Room& room : rooms) {

        json roomJson = room.toJSON();
        data["rooms"].push_back(roomJson);
    }
    writeToFile(data);
}

bool RoomHandler::checkIfRoomNameOpen(std::string room_name){

    if (Rooms.size() == 0){
        return false;
    }
    for (Room& room : Rooms) {
        std::string name = room.getRoomName();
        if (name == room_name){
            return true;
        }      
    }
    return false;
}

void RoomHandler::handleRoomCreate(json data,int clientsocket){
    if (data["name"] != nullptr){
        mutexRooms.lock();
        std::string room_name = data["name"];
        std::cout << room_name << std::endl;
        json response;
        bool ifExist = checkIfRoomNameOpen(room_name);
        if (ifExist == true){
        response["status"] = "failure";
        response["type"] = "room_create";
        std::string responseStr = response.dump();
        clientHandler.sendToClient(clientsocket,responseStr);
        mutexRooms.unlock();
        return;
        }
        else{
            response["status"] = "succes";
            response["type"] = "room_create";
        }
        Room newRoom(room_name);
        newRoom.addPlayer(clientsocket,clientInfoMap);
        newRoom.setGameMaster(clientInfoMap[clientsocket].nick);
        Rooms.push_back(newRoom);
        std::cout << "Here room should be stored in json" << std::endl;
        roomsToFile(Rooms);
        
        response["room_name"] = newRoom.name;
        response["players"] = newRoom.players;
        response["gameMaster"] = newRoom.getGameMaster();
        std::string responseStr = response.dump();
        // TUUTAJ ZROBIĆ COŚ Z WYSYŁNIAME DO KLIENTÓW
        clientHandler.sendToAllClients(responseStr);
        mutexRooms.unlock();

        std::thread([this, roomName = newRoom.getRoomName()]() {
            this->handleRoom(roomName);
            }).detach();
        
    }
    else{
        std::cout << "Room name is equall to null" << std::endl;
    }
}


void RoomHandler::RemovePlayerFromRoom(json data,int clientsocket){
    std::string room_name = data["name"];
    mutexRooms.lock();
    for (Room& room : Rooms) {
            std::string name = room.getRoomName();
            if (name == room_name){
                mutexClientInfoMap.lock();
                room.removePlayer(clientsocket,clientInfoMap);
                std::cout << "Player removed from a lobby,number of players: " << room.getNumberOfPlayers() << std::endl;
                mutexClientInfoMap.unlock();
                checkIfGameMaster(clientsocket,room);
                roomsToFile(Rooms);
                //Znowu wysyłanie wiadomości
                clientHandler.sendToClientsRoomsInfo(clientsocket);
            }
    
        }
    mutexRooms.unlock(); 
}

void RoomHandler::checkIfGameMaster(int clientsocket,Room& room){
    std::string player = room.getGameMaster();
    std::cout << "Current Game master: " << player << std::endl;
    mutexClientInfoMap.lock();
    if (player == clientInfoMap[clientsocket].nick){
        std::string newGameMaster = room.getNewGameMaster();
        std::cout << "New Game master: " << newGameMaster << std::endl;
        if (newGameMaster != "No players in room"){
            room.setGameMaster(newGameMaster);
        }
        else {
            room.setGameMaster("");
        }
        
    }
    mutexClientInfoMap.unlock();  
}

void RoomHandler::handlePlayer(json data,int clientsocket){
    std::string room_name = data["name"];
    mutexRooms.lock();
    
    for (Room& room : Rooms) {
        std::string name = room.getRoomName();
        if (name == room_name){ 
            json response;
            response["type"] = "player_join_room";
            response["room_name"] = room.name;
            mutexClientInfoMap.lock();
            if (room.getNumberOfPlayers() >= 5){
                response["status"] = "failure";
            }
            else{
                response["status"] = "succes";
                room.addPlayer(clientsocket,clientInfoMap);
                std::cout << "Player joined a lobby,number of players: " << room.getNumberOfPlayers() << std::endl;
                if (room.getGameMaster() == ""){
                    room.setGameMaster(clientInfoMap[clientsocket].nick);
                }
                mutexClientInfoMap.unlock();
                
                roomsToFile(Rooms);
                clientHandler.sendToClientsRoomsInfo(clientsocket);
            }
            std::string responseStr = response.dump();
            clientHandler.sendToClient(clientsocket,responseStr);
           
        }   
    }
    mutexRooms.unlock();
    
}

void RoomHandler::StartGame(json data,int clientsocket){

    std::ifstream questionsFile("serverJSONs/questions.json");
    json questionsJson;
    questionsFile >> questionsJson;
    // print entire json
    std::cout << questionsJson["categories"] << std::endl;



    mutexRooms.lock();
    std::string room_name = data["name"];
    std::string category = data["category"];
    std::string savePoints = data["save_points"];
    toLowerCase(category); // in json categories are stored in lowercase
    std::cout << "Category: " << category << std::endl;


    if (questionsJson["categories"].contains(category)) {
        json categoryQuestions = questionsJson["categories"][category];

        for (Room& room : Rooms) {
                std::string name = room.getRoomName();
                if (name == room_name){
                    if (savePoints == "No"){
                        room.setZeroPlayerPoints();
                    }

                    room.setStatus("Started");
                    room.setCategory(category);
                    //  std::cout << categoryQuestions[0]["questionNumber"] <<", "<< categoryQuestions[0]["question"] << ", " << categoryQuestions[0]["options"] << categoryQuestions[0]["correctAnswer"] << std::endl;
                    std::cout << "Category questions: " << categoryQuestions << std::endl;

                    int categorySize = categoryQuestions.size();
                    if(room.getMaxQustions() > categorySize){
                        room.setMaxQuestions(categorySize); 
                        // Error prevention, if max questions is greater than category size, set max questions to category size
                    }
                    room.resetQuestionIndices(categorySize);

                    std::random_device dev;
                    std::mt19937 rng(dev());
                    std::uniform_int_distribution<std::mt19937::result_type> dist(0,categorySize-1);
                    int randomIndex = dist(rng);
                    room.setIndex(1);
                    room.setCurrentQuestion(categoryQuestions[randomIndex]["questionId"],categoryQuestions[randomIndex]["questionText"],categoryQuestions[randomIndex]["options"],categoryQuestions[randomIndex]["correctAnswer"]);
                    room.setTimeStampQuestionUpdate(std::chrono::system_clock::now());
                    roomsToFile(Rooms);
                    clientHandler.sendToClientsRoomsInfo(clientsocket);

                    mutexRooms.unlock(); // after room is found and started, unlock mutex and exit function
                    return;
                }
                
            }
    }
    mutexRooms.unlock(); 
}

void RoomHandler::toLowerCase(std::string &str){
    std::transform(str.begin(),str.end(),str.begin(),::tolower);
}