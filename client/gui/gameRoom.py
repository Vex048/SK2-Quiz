import tkinter as tk

class GameRoom(tk.Frame):
    def __init__(self,root,socket):
        super().__init__(root)
        tk.Label(self, text="GameRoom", font=("Calibri", 24)).pack(pady=20)
        tk.Button(self,text="Start a game",command=self.connected).pack()
        self.socket=socket
    def connected(self):
        self.socket.send(f"Server connected a player to game room".encode())
        
    
