#include "RoomHandler.h"
#include "ClientHandler.h"



// Method that start with thread after creating a new Room by client 
void RoomHandler::handleRoom(std::string room_name){
    //Downloading a questions from questions.json
    std::ifstream questionsFile("serverJSONs/questions.json");
    json questionsJson;
    questionsFile >> questionsJson;
    // Putting condition variable lock on
    std::unique_lock<std::mutex> lock(roomCVMutex);  
    auto& cv = roomCVMap[room_name]; 
    while(true){ 
        //sleep(1);
        // Using cv.wait_for, beocuse this method has to constantly check if the room is empty
        
        bool notified = cv.wait_for(lock, std::chrono::seconds(1), [&]() {
            //returning true if some event like player joined this room, player exit room, game start or answer from client occured
            //otherwise false 
            return roomEventOccurred(room_name);  
        }); 
        // Locking possibility to change room values
        mutexRooms.lock();
        Room* room = getRoomFromFile(room_name);
        if (room==nullptr){
            mutexRooms.unlock();
            break;
        }

        // If an event occured or game is started procces the game logic 
        if (notified || room->getStatus() == "Started") {
            // Handle any events (e.g., a player joined, game started, etc.)
            std::cout << "Room " << room_name << " was notified of an event." << std::endl;
            Room* room = getRoomFromFile(room_name);
            if (room != nullptr && room->getStatus() == "Started") {
                processGameEvent(room, questionsJson);
            }
            // And set event flag to false
            roomFlagsEvents[room_name] = false;
        }
        // Getting Number of players in room
        int lastNumberOfPlayers = room->getNumberOfPlayers();        
        std::cout << "Number of players in room: " << lastNumberOfPlayers << std::endl;

        // If there are no players in room start counting down to delete room
        if(lastNumberOfPlayers==0){        
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = now - room->getTimeStampPlayerLeftRoom();
            std::cout << "Elapsed time room empty: " << elapsed_seconds.count() << "s\n";
            if (elapsed_seconds.count() > 10){ 
                std::cout << "Room is empty for more than 1 minute, deleting room" << std::endl;
                //Removing room and updateing it to rooms.json
                removeRoom(room_name);
                roomsToFile(Rooms);
                mutexRoomClients.lock();
                // Clearing structures
                roomClients.erase(room_name);
                mutexRoomClients.unlock();               
                roomFlagsEvents.erase(room_name);
                roomCVMap.erase(room_name);
                // Send informations to clients in lobby about room removal
                clientHandler.sendToLobbyClientsRoomsInfo();
                mutexRooms.unlock();
                break;
            }
        }
        
        mutexRooms.unlock();
    }
}

// Checkicng a roomFlagEvents for an event in a room
bool RoomHandler::roomEventOccurred(std::string room_name){
    return roomFlagsEvents[room_name];
}

// The quiz logic 
void RoomHandler::processGameEvent(Room* room,json questionsJson){
    // Getting a category 
    std::string RoomCategory = room->getCategory();
    // Start counting time for one question
    std::chrono::time_point<std::chrono::system_clock> now1 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds2 = now1 - room->getTimeStampPlayerQuestionUpdate();
    std::cout << "Elapsed time for question: " << elapsed_seconds2.count() << "s\n";
    // Check if 15 seconds passed or everyone has answered 
    //if (elapsed_seconds2.count() > 15 || room->curQuestion.numOfAnswers == room->getNumberOfPlayers()){
    if (elapsed_seconds2.count() > 15 ){ 
        std::cout << "15 second for question has finished" << std::endl;
        
        // Get questions from json
        json categoryQuestions = questionsJson["categories"][RoomCategory];

        //Sending correct answer to clients
        json response;
        response["type"] = "answer_to_cur_question";
        json quest;
        quest["correctAnswer"] = room->curQuestion.correctAnswer;
        quest["questionText"] = room->curQuestion.questionText;
        quest["options"] = room->curQuestion.options;
        quest["questionId"] = room->curQuestion.questionId;
        response["data"] = quest;
        std::string responseStr = response.dump();
        responseStr = responseStr + "\n";
        room->sendToClientsInRoom(responseStr,nicknameToSocket);
        // Sleep is for clients to see if they did have correct answer or not
        sleep(3);


        // Prepare a random number 
        std::random_device dev;
        std::mt19937 rng(dev());
        int index = room->getIndex();
        // If index is equals 0 then end a game
        if (index == 0){
            std::cout << "Game is finished" << std::endl;
            json response;                    
            response["type"] = "game_finished";
            // Get points
            json scores = room->getAllPoints(clientInfoMap);
            std::string scoresStr = scores.dump();
            scoresStr = scoresStr + "\n";
            std:: cout << scoresStr << std::endl;
            response["scores"] = scores;
            std::string responseStr = response.dump();
            responseStr = responseStr + "\n";
            // Send points to all clients in room and set a status to waiting
            clientHandler.sendToRoomClientsRoomsInfo(room->getRoomName());
            room->sendToClientsInRoom(responseStr,nicknameToSocket);
            room->setStatus("Waiting");    
            roomFlagsEvents[room->getRoomName()] = true;
            roomsToFile(Rooms);
            // send to lobby clients information that game ended
            clientHandler.sendToLobbyClientsRoomsInfo();

        }
        else {       
            // If game didnt end get a random number        
        std::uniform_int_distribution<std::mt19937::result_type> dist(0,room->questionIndices.size()-1);
        int randomIndex = dist(rng);
        randomIndex = room->questionIndices[randomIndex];
        // Set new questions index and questions
        room->setIndex(index+1);
        room->setCurrentQuestion(categoryQuestions[randomIndex]["questionId"],
            categoryQuestions[randomIndex]["questionText"],categoryQuestions[randomIndex]["options"],
            categoryQuestions[randomIndex]["correctAnswer"]);
        //Update a clock 
        room->setTimeStampQuestionUpdate(std::chrono::system_clock::now());                   
        roomsToFile(Rooms); 
        json response;

        // Send new question to all clients in the game room 
        response["type"] = "new_question";
        json quest;
        quest["questionText"] = categoryQuestions[randomIndex]["questionText"];
        quest["options"] = categoryQuestions[randomIndex]["options"];
        quest["questionId"] = categoryQuestions[randomIndex]["questionId"];
        response["data"] = quest;
        std::string responseStr = response.dump();
        responseStr = responseStr + "\n";
        room->sendToClientsInRoom(responseStr,nicknameToSocket);

  
        }                
    }
}

// Method that remomve Room after 1 minute of inactivity
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
// Method that return a room pointer from a rooms.json
Room* RoomHandler::getRoomFromFile(std::string room_name){
    for (Room& room : Rooms) {
        std::string name = room.getRoomName();
        if (name == room_name){
            return &room;
        }      
    }
    return nullptr;
}
// Write to file rooms.json
void RoomHandler::writeToFile(json data){
    std::ofstream o("serverJSONs/rooms.json");
    if (!o) {
        std::cerr << "Error: Unable to open file for writing.\n";
        return;
    }
    o << std::setw(4) << data << std::endl; // Pretty print with indentation
    o.close();
}
// Get current rooms to file
void RoomHandler::roomsToFile(std::vector<Room>& rooms){
    json data;
    data["rooms"] = json::array();
     for (const Room& room : rooms) {

        json roomJson = room.toJSON();
        data["rooms"].push_back(roomJson);
    }
    writeToFile(data);
}
// Check if room name is already taken
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

// Method invoked when client creates a new room 
void RoomHandler::handleRoomCreate(json data,int clientsocket){
    if (data["name"] != nullptr){
        mutexRooms.lock();
        std::string room_name = data["name"];
        std::cout << room_name << std::endl;
        json response;
        // Search for an existing name
        bool ifExist = checkIfRoomNameOpen(room_name);
        if (ifExist == true){
            // If there is a room with that name send a failure
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
        // Create new room and add a player that created this room to it
        Room newRoom(room_name);
        newRoom.addPlayer(clientsocket,clientInfoMap);
        // make him Game master
        newRoom.setGameMaster(clientInfoMap[clientsocket].nick);
        Rooms.push_back(newRoom);
        std::cout << "Here room should be stored in json" << std::endl;
        roomsToFile(Rooms);
        
        response["room_name"] = newRoom.name;
        response["players"] = newRoom.players;
        response["gameMaster"] = newRoom.getGameMaster();
        std::string responseStr = response.dump();


        mutexLobbyClients.lock();
        lobbyClients.erase(clientsocket);
        mutexLobbyClients.unlock();

        mutexRoomClients.lock();
        roomClients[room_name].insert(clientsocket);
        mutexRoomClients.unlock();

        // Send informatiosn about new room to all clients in lobby and to clients in this room (to a client that created a room)
        clientHandler.sendToLobbyClientsRoomsInfo(responseStr);
        clientHandler.sendToRoomClientsRoomsInfo(responseStr,room_name);
        //clientHandler.sendToAllClients(responseStr);
        mutexRooms.unlock();

        // Detach new thread that handle a game and room logic
        std::thread([this, roomName = newRoom.getRoomName()]() {
            this->handleRoom(roomName);
            }).detach();
        
    }
    else{
        std::cout << "Room name is equall to null" << std::endl;
    }
}

// Remove player from a room 
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
                // Check if he is a game master
                // If he is then method below select a new one 
                checkIfGameMaster(clientsocket,room);
                roomsToFile(Rooms);

                mutexLobbyClients.lock();
                lobbyClients.insert(clientsocket);
                mutexLobbyClients.unlock();

                mutexRoomClients.lock();
                roomClients[room_name].erase(clientsocket);
                mutexRoomClients.unlock();

                // Send to client in lobby and this room updated information 
                clientHandler.sendToLobbyClientsRoomsInfo();
                if (room.getStatus() != "Started"){
                    clientHandler.sendToRoomClientsRoomsInfo(name);
                }
                
            }
    
        }
    mutexRooms.unlock(); 
}

// Check if there is a game master, if not select one 
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


// Method that handles joining player to a room
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
            // If there is 5 players (or somehow more) in room player cannot join 
            // This handle a situation where players try to join the same room in the exact same moment when threre are 4 players in room
            // One will be able to pass, but other will get rejected
            if (room.getNumberOfPlayers() >= 5){
                response["status"] = "failure";
            }
            else{
                response["status"] = "succes";
                room.addPlayer(clientsocket,clientInfoMap);
                std::cout << "Player joined a lobby,number of players: " << room.getNumberOfPlayers() << std::endl;
                // If there is no game master, new player become one
                if (room.getGameMaster() == ""){
                    room.setGameMaster(clientInfoMap[clientsocket].nick);
                }
                mutexClientInfoMap.unlock();

                mutexLobbyClients.lock();
                lobbyClients.erase(clientsocket);
                mutexLobbyClients.unlock();

                mutexRoomClients.lock();
                roomClients[room_name].insert(clientsocket);
                mutexRoomClients.unlock();

                roomsToFile(Rooms);

                // Update room info to clients in the same room and in lobby
                clientHandler.sendToLobbyClientsRoomsInfo();
                clientHandler.sendToRoomClientsRoomsInfo(name);
            }
            std::string responseStr = response.dump();
            // Send info to client if he was able to join room or not
            clientHandler.sendToClient(clientsocket,responseStr);
           
        }   
    }
    mutexRooms.unlock();
    
}

// Method that handle Game start by a game master
void RoomHandler::StartGame(json data){
    // Getting questions from questions.json
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
                    // Check if game master want to save points from previous round or not
                    if (savePoints == "No"){
                        room.setZeroPlayerPoints();
                    }

                    room.setStatus("Started");
                    room.setCategory(category);
                    //std::cout << "Category questions: " << categoryQuestions << std::endl;

                    int categorySize = categoryQuestions.size();
                    if(room.getMaxQustions() > categorySize){
                        room.setMaxQuestions(categorySize); 
                        // Error prevention, if max questions is greater than category size, set max questions to category size
                    }
                    room.resetQuestionIndices(categorySize);
                    // Creating random distribution
                    std::random_device dev;
                    std::mt19937 rng(dev());
                    //Getting random question 
                    std::uniform_int_distribution<std::mt19937::result_type> dist(0,categorySize-1);
                    int randomIndex = dist(rng);
                    //Seting question index to one, becouse its the first question
                    room.setIndex(1);
                    // Setting current question and settign a timestamp
                    room.setCurrentQuestion(categoryQuestions[randomIndex]["questionId"],categoryQuestions[randomIndex]["questionText"],categoryQuestions[randomIndex]["options"],categoryQuestions[randomIndex]["correctAnswer"]);
                    room.setTimeStampQuestionUpdate(std::chrono::system_clock::now());
                    roomsToFile(Rooms);
                    // Sending informations about a game start to clients in lobby and to clients in this game room
                    clientHandler.sendToLobbyClientsRoomsInfo();
                    clientHandler.sendToRoomClientsRoomsInfo(name);

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