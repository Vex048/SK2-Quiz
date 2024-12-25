import tkinter as tk
import json
class GameRoom(tk.Frame):
    def __init__(self,masterRoot,FrameManager,socket):
        super().__init__(masterRoot)
        self.socket=socket
        self.frameManager=FrameManager
        self.roomName = "deafult"
        tk.Label(self, text="GameRoom", font=("Calibri", 24)).pack(pady=20)
        self.labelName = tk.Label(self, text=self.roomName, font=("Arial", 18))
        self.labelName.pack(pady=20)
        tk.Button(self,text="Start a game",command=self.connected).pack()
        tk.Button(self,text="Exit Room",command=self.exitRoom).pack()
        
        self.players_listbox = tk.Listbox(self)
        self.players_listbox.pack(fill="both", expand=True)
        self.socket=socket


    def setRoomName(self,name):
        self.roomName = name
        self.labelName["text"] = self.roomName
    def addPlayerListbox(self,players):
        self.players_listbox.delete(0, "end")
        for player in players:
             self.players_listbox.insert("end", player)

    def exitRoom(self):
        message = {
            "type": "player_exit_room",
            "name": self.roomName
        }
        jsonStringRoom = json.dumps(message)
        print(jsonStringRoom)
        self.socket.send(jsonStringRoom.encode("utf-8"))
        self.frameManager.showFrame("Lobby")
    def connected(self):
        pass
    def playerConnected(self):
        pass


    # def listenForServerUpdates(self):
    #     while True:
    #         try:
    #             message = self.socket.recv(1024).decode()
    #             if not message:  
    #                 print("Connection closed by the server")
    #                 break
    #             update = json.loads(message)  
    #             self.handleUpdate(update)
    #         except json.JSONDecodeError as e:
    #             print(f"JSON decode error: {e}, received message: {message}")
    #             break
    #         except Exception as e:
    #             print(f"Error: {e}")
    #             break

    # def handleUpdate(self,update):
    #     if update['type'] == "room_create":
    #         room_name = update["room_name"]
    #         players=update['players']
    #         d = {"name":room_name,"players":players,"status":"Waiting"}
    #         self.rooms.append(d)
    #         self.refresh_rooms()
        
    
