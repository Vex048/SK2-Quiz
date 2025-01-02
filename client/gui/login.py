import tkinter as tk
import json
from tkinter import messagebox
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
        if name == "":
            messagebox.showerror("Name error", "You cant enter a game as empty space.")
            return
        self.sendNickToserver(name)


    def sendNickToserver(self,nick):
        nickname = {
            "type": "create_nickname",
            "nickname": nick
        }
        jsonString = json.dumps(nickname) + "\n"
        self.socket.send(jsonString.encode("utf-8"))

    def handleUpdateNick(self,update):
        if update["type"] == "create_nickname":
            if update["status"] == "succes":
                self.frameManager.setNickname(update["nickname"])
                self.frameManager.showFrame("Lobby")
            else:
                messagebox.showerror("Name error", "Nick already taken. Please choose another")




    
        
