import tkinter as tk

class Login(tk.Frame):
    def __init__(self,masterRoot,FrameManager,socket):
        super().__init__(masterRoot)
        self.socket=socket
        self.frameManager=FrameManager
        self.rooms=[]
        tk.Label(self, text="Login", font=("Arial", 24)).pack(pady=20)
        self.nick_entry = tk.Entry(self, width=20)
        self.nick_entry.pack(pady=10)
        tk.Button(self, text="Connect", command=self.login).pack(pady=10)

    def login(self):
        name = self.nick_entry.get()
        print(f"Próba zalogowania jako: {name}")
        self.sendNickToserver(name)
        self.frameManager.showFrame("Lobby")


    def sendNickToserver(self,nick):
        self.socket.send(f"NICK|{nick}".encode())
        
