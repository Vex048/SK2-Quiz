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


    def initialize(self):
        self.rooms_tree = ttk.Treeview(self, columns=("Players", "Status"), show="headings", height=15)
        self.rooms_tree.heading("Players", text="Players")
        self.rooms_tree.heading("Status", text="Status")
        self.rooms_tree.pack(pady=10, padx=20)
        tk.Button(self, text="Create Room", command=self.create_room).pack(side="left", padx=10)
        tk.Button(self, text="Join Room", command=self.join_room).pack(side="left", padx=10)


    def create_room(self):
        # Push a info to server
        room_name = tk.simpledialog.askstring("Create Room", "Enter room name:")
        messagebox.showinfo("Success", f"Room '{room_name}' created!")
        message = {
            "type": "create_room",
            "name": room_name
        }
        jsonStringRoom = json.dumps(message)
        print(jsonStringRoom)
        self.socket.send(jsonStringRoom.encode("utf-8"))
        listenForServerUpdate = threading.Thread(target=self.listenForServerUpdates)
        listenForServerUpdate.start()
        self.refresh_rooms()

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
        print(f"Joining room: {room_name}")
        self.frameManager.showFrame("GameRoom")
        self.frameManager.frames['GameRoom'].playerConnected()

    def refresh_rooms(self):
        # self.rooms = [
        #     {"name": "Room1", "players": 1, "status": "Waiting"},
        #     {"name": "Room2", "players": 5, "status": "Started"},
        # ]

        for row in self.rooms_tree.get_children():
            self.rooms_tree.delete(row)

        for room in self.rooms:
            self.rooms_tree.insert("", "end", values=(f"{room['name']} ({room['players']}/5)", room['status']))


    def listenForServerUpdates(self):
        while True:
            try:
                message = self.socket.recv(1024).decode()
                if not message:  
                    print("Connection closed by the server")
                    break
                update = json.loads(message)  
                self.handleUpdate(update)
            except json.JSONDecodeError as e:
                print(f"JSON decode error: {e}, received message: {message}")
                break
            except Exception as e:
                print(f"Error: {e}")
                break

    def handleUpdate(self,update):
        print(update)
        if update['type'] == "room_create":
            room_name = update["room_name"]
            players=update['players']
            d = {"name":room_name,"players":players,"status":"Waiting"}
            self.rooms.append(d)
            self.refresh_rooms()
        elif update['type'] == "room_update":
            room_name = update["room_name"]
            players=update['players']
            self.rooms[room_name]["players"] = players
            self.refresh_rooms()





