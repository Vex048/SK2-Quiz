import tkinter as tk
from tkinter import ttk
from tkinter import simpledialog,messagebox
import json
import threading
global games 
games =3
class Lobby(tk.Frame):
    def __init__(self,masterRoot,FrameManager,socket):
        self.buttons=[]
        self.socket = socket
        super().__init__(masterRoot)
        self.socket=socket
        self.frameManager=FrameManager
        self.rooms = [] # Get it from JSON
        tk.Label(self, text="Lobby", font=("Arial", 18)).pack(pady=20)
        self.initialize()

    def getCurrentRooms(self):
        message = {
            "type": "rooms_info"
        }
        jsonStringRoom = json.dumps(message) + "\n"
        self.socket.send(jsonStringRoom.encode("utf-8"))

    def initialize(self):
        self.rooms_tree = ttk.Treeview(self, columns=("Players", "Status"), show="headings", height=10)
        self.rooms_tree.heading("Players", text="Players")
        self.rooms_tree.heading("Status", text="Status")
        self.rooms_tree.pack(pady=5, padx=10)
        buttonFrame = tk.Frame(self)
        buttonFrame.pack(pady=10)
        tk.Button(buttonFrame, text="Create Room", command=self.create_room).pack(side="left", padx=10)
        tk.Button(buttonFrame, text="Join Room", command=self.join_room).pack(side="left", padx=10)
        tk.Button(buttonFrame, text="Refresh", command=self.getCurrentRooms).pack(side="left", padx=10)
        self.getCurrentRooms()


    def create_room(self):
        # Push a info to server
        room_name = tk.simpledialog.askstring("Create Room", "Enter room name:")
        if room_name is None:
            return
        
        message = {
            "type": "create_room",
            "name": room_name
        }
        jsonStringRoom = json.dumps(message) + "\n"
        self.socket.send(jsonStringRoom.encode("utf-8"))
        # self.refresh_rooms()
        # messagebox.showinfo("Success", f"Room '{room_name}' created!")  
        # self.frameManager.frames["GameRoom"].setRoomName(room_name)
        # self.frameManager.showFrame("GameRoom")
        

    def join_room(self):
        # Push a info to server
        selected = self.rooms_tree.focus()
        if not selected:
            messagebox.showwarning("No Selection", "Please select a room to join.")
            return
        room_name = self.rooms_tree.item(selected, "values")[0].split(" (")[0]
        room_status = self.rooms_tree.item(selected, "values")[1]      
        if room_status == "Started":
            messagebox.showerror("Error", "Cannot join a game that has already started.")
            return
        for room in self.rooms:
            if room["name"] == room_name:
                if len(room["players"]) == 5:
                    messagebox.showerror("Error", "Room is full.")
                    return
    
        print(f"Joining room: {room_name}")
        message = {
            "type": "player_join_room",
            "name": room_name
        }
        jsonStringPlayer = json.dumps(message) + "\n"
        self.socket.send(jsonStringPlayer.encode("utf-8"))
        #self.getCurrentRooms()
        # self.frameManager.frames["GameRoom"].setRoomName(room_name)
        # self.frameManager.showFrame("GameRoom")

    def getPlayersFromRoom(self,roomname):
        players = self.rooms[roomname]["players"]
        return players


    def refresh_rooms(self):
        for row in self.rooms_tree.get_children():
            self.rooms_tree.delete(row)
        for room in self.rooms:
            self.rooms_tree.insert("", "end", values=(f"{room['name']} ({len(room['players'])}/5)", room['status']))

    def handleUpdate(self,update):
        if update['type'] == "room_create":
            if update['status'] == "succes":
                room_name = update["room_name"]
                players=update['players']
                gameMaster = update['gameMaster']
                d = {"name":room_name,"players":players,"status":"Waiting","gameMaster":gameMaster}
                self.rooms.append(d)
                for room in self.rooms:
                    if self.frameManager.getNick() in room["players"]:
                        if self.frameManager.getNick() == gameMaster:
                            messagebox.showinfo("Succes", f"Room '{room_name}' created!")
                            self.frameManager.frames["GameRoom"].setRoomName(room_name)
                            self.frameManager.showFrame("GameRoom")
                            self.frameManager.frames["GameRoom"].isGameMaster=True 
                            self.frameManager.frames["GameRoom"].updateGameMasterButton()                                    
                        self.frameManager.frames["GameRoom"].addPlayerListbox(room["players"])
                self.refresh_rooms()
            else:
                messagebox.showerror("Error", "Couldnt create a room, try other room name")

        elif update['type'] == "rooms_info":
            if update["status"] == "succes":
                self.rooms=[]
                for room in update["rooms"]:
                    temp = {"name":room["name"],"players":room["players"],"status":room["status"],"gameMaster":room["gameMaster"]}
                    if self.frameManager.getNick() in temp["players"]:
                        if temp["gameMaster"] == self.frameManager.getNick():
                            self.frameManager.frames["GameRoom"].isGameMaster=True
                            self.frameManager.frames["GameRoom"].updateGameMasterButton()
                        self.frameManager.frames["GameRoom"].addPlayerListbox(room["players"])
                        if temp["status"] == "Started":
                            self.frameManager.frames["QuizView"].setRoomName(room["name"])
                            print(room["questionInfo"])
                            self.frameManager.frames["QuizView"].update_question(room["questionInfo"])
                            self.frameManager.showFrame("QuizView")
                    self.rooms.append(temp)
                self.refresh_rooms()
            else:
                self.rooms=[]
                self.refresh_rooms()
        elif update['type'] == "player_join_room":
            if update["status"] == "succes":
                room_name = update["room_name"]
                self.getCurrentRooms()
                self.frameManager.frames["GameRoom"].setRoomName(room_name)
                self.frameManager.showFrame("GameRoom")
            else:
                messagebox.showerror("Error", "Trouble with joining to room, please try again")
        else:
            self.rooms=[]
            self.refresh_rooms()
            print("No rooms name")







