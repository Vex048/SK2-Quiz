import tkinter as tk

global games 
games =3
class Lobby(tk.Frame):
    def __init__(self,masterRoot,socket):
        self.buttons=[]
        self.socket = socket
        super().__init__(masterRoot)
        tk.Label(self, text="Lobby", font=("Arial", 18)).pack(pady=20)
        for i in range(games):
            self.buttons.append(tk.Button(self, text="Join Lobby" + str(i+1), command=lambda i=i: self.connectToRoom(i)).pack(pady=10))




    def connectToRoom(self,room):
        print(f"Connect to room: {room}")
        self.socket.send(f"JOIN |  joined room:{room}".encode())
