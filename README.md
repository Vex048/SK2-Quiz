# Multiplayer Quiz Game

## Description
This project is a multiplayer quiz game implemented in C++ and Python Players can join different rooms, select quiz categories, and compete with others in a real-time trivia challenge.

## Features
- **User Login:** Players enter a nickname upon connecting to the server. If the nickname is already taken, they must choose another.
- **Lobby System:** Players can see available rooms, the number of players in each room, and whether a game is in progress.
- **Room Management:**
  - Maximum of 5 players per room.
  - Players can create new rooms or join only those where the game hasn't started yet.
  - Empty rooms are deleted after 5 minutes.
- **Quiz Categories:**
  - Players can choose from 5 different categories: *Sports, History, Geography, Culture, and Music.*
- **Game Flow:**
  - The longest waiting player in the room selects the quiz category.
  - The server sends questions to all players simultaneously.
  - Players have **15 seconds** to answer each question.
  - Points are assigned based on the correctness
  - A **leaderboard** is updated and displayed after each question.
- **Game End:**
  - The game ends after **15 questions** or when all players leave the room.
  - A **final scoreboard** is shown, listing the scores of all players.
  - Players are returned to the **lobby**, where they can choose to retain or reset their score for the next game.
- **Question Storage:** Questions are manually created and stored in **JSON files**.

## Technologies Used
- **C++** for server implementation
- **Python** for client implementation (tkinter library for GUI)
- **Sockets** for network communication
- **Threads** for handling multiple players and rooms
- **JSON** for storing quiz questions


## How to Run
1. Clone the repository
```
  clone https://github.com/Vex048/SK2-Quiz.git
  cd SK2-Quiz
```
2. Build the project using cmake
``` cmake
   mkdir build && cd build
   cmake ..
   make
```   
3. Start the server:
``` 
   ./SK2-Quiz-Server
```
4. Run the client:
* If you don't have the Tkinter library installed, first install it using pip:
``` 
    pip install tkinter
```
* Starting from the main directory:
```
  cd client
  python3.12 client.py
```


## Authors
*  **[Wiktor](https://github.com/veektorf1)**
*  **[Marcin](https://github.com/Vex048)**


