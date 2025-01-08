import tkinter as tk
from tkinter import ttk 
import json
class GameRoom(tk.Frame):
    def __init__(self,masterRoot,FrameManager,socket):
        super().__init__(masterRoot)
        self.socket=socket
        self.frameManager=FrameManager
        self.roomName = "deafult"
        n = tk.StringVar()
        self.categories = ttk.Combobox(self,width=27,textvariable=n)
        self.categories["values"]= ("Sports","History","Geography","Music","Cultural")
        #categories.grid(column = 1, row = 5) 
        self.isGameMaster=False
        
        tk.Label(self, text="GameRoom", font=("Calibri", 24)).pack(pady=20)
        self.labelName = tk.Label(self, text=self.roomName, font=("Arial", 18))
        self.labelName.pack(pady=20)
        tk.Label(self, text="Please select quiz categroy", font=("Calibri", 12)).pack(pady=20)
        
        # self.categories.pack()
        # self.categories.current(1)
        self.gameMasterButton = tk.Button(self,text="Start a game",command=self.gameStart)

        self.Checkbutton2=tk.IntVar()
        self.checkButtonSavePoints = tk.Checkbutton(self, text="Save points from previous game", 
        variable=self.Checkbutton2, onvalue=1, offvalue=0)
        self.updateGameMasterButton()
        tk.Button(self,text="Exit Room",command=self.exitRoom).pack()
        
        self.players_listbox = tk.Listbox(self)
        self.players_listbox.pack(fill="both", expand=True)
        self.socket=socket

    def updateGameMasterButton(self):
        if self.isGameMaster == True:
            self.gameMasterButton.pack()
            self.categories.pack()
            #self.categories.current(1)
            self.checkButtonSavePoints.pack()
        else:
            self.gameMasterButton.pack_forget()
            self.categories.pack_forget()
            self.checkButtonSavePoints.pack_forget()

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
        if self.isGameMaster ==True:
            self.isGameMaster = False
            self.updateGameMasterButton()
        
        jsonStringRoom = json.dumps(message) + "\n"
        self.socket.send(jsonStringRoom.encode("utf-8"))
        self.frameManager.showFrame("Lobby")
    def getCheckboxCheck(self):
        num = self.Checkbutton2.get()
        if num == 1:
            return "Yes"
        else:
            return "No"
    def gameStart(self):
        if self.categories.get() == "":
            print("Pusto")
            return
        print(self.categories.get())
        message = {
            "type":"start_game",
            "name":self.roomName,
            "category": self.categories.get(),
            "save_points": self.getCheckboxCheck()
        }
        jsonStringRoom = json.dumps(message) + "\n"
        self.socket.send(jsonStringRoom.encode("utf-8"))
    def playerConnected(self):
        pass



        
    
