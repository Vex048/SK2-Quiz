import tkinter as tk

class GameRoom(tk.Frame):
    def __init__(self,masterRoot,FrameManager,socket):
        super().__init__(masterRoot)
        self.socket=socket
        self.frameManager=FrameManager
        tk.Label(self, text="GameRoom", font=("Calibri", 24)).pack(pady=20)
        tk.Button(self,text="Start a game",command=self.connected).pack()
        self.socket=socket
    def connected(self):
        self.socket.send(f"Player wants to start a game".encode())
        
    
