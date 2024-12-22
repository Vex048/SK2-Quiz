import tkinter as tk

class Login(tk.Frame):
    def __init__(self,masterRoot,lobbyframe,socket):
        super().__init__(masterRoot)
        self.socket=socket
        tk.Label(self, text="Login", font=("Arial", 24)).pack(pady=20)
        self.nick_entry = tk.Entry(self, width=20)
        self.nick_entry.pack(pady=10)
        tk.Button(self, text="Connect", command=lambda: self.login(lobbyframe)).pack(pady=10)

    def login(self,lobbyframe):
        name = self.nick_entry.get()
        print(f"Próba zalogowania jako: {name}")
        self.sendNickToserver(name)
        lobbyframe.tkraise()


    def sendNickToserver(self,nick):
        self.socket.send(f"NICK|{nick}".encode())
        
