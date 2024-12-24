import tkinter as tk
import json
class GameRoom(tk.Frame):
    def __init__(self,masterRoot,FrameManager,socket):
        super().__init__(masterRoot)
        self.socket=socket
        self.frameManager=FrameManager
        self.rootName = ""
        tk.Label(self, text="GameRoom", font=("Calibri", 24)).pack(pady=20)
        tk.Label(self, text=self.rootName, font=("Arial", 18)).pack(pady=20)
        tk.Button(self,text="Start a game",command=self.connected).pack()
        
        self.players_listbox = tk.Listbox(self)
        self.players_listbox.pack(fill="both", expand=True)
        self.socket=socket
    
    def addPlayerListbox(self,players):
        self.players_listbox.delete(0, "end")
        for player in players:
             self.players_listbox.insert("end", player)

    def connected(self):
        pass
    def playerConnected(self):
        pass
    def setName(self,name):
        self.roomName = name

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
        if update['type'] == "room_create":
            room_name = update["room_name"]
            players=update['players']
            d = {"name":room_name,"players":players,"status":"Waiting"}
            self.rooms.append(d)
            self.refresh_rooms()
        
    
