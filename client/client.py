import socket
import threading
import tkinter as tk
from tkinter import messagebox
from gui.login import Login
from gui.lobby import Lobby
from gui.gameRoom import GameRoom
from gui.frameManager import FrameManager 
SERVER_IP = "127.0.0.1"
SERVER_PORT = 8080

class QuizClient:
    def __init__(self, root):
        self.root = root
        self.root.title("Quiz Game")
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.client_socket.connect((SERVER_IP, SERVER_PORT))
<<<<<<< HEAD
=======
        self.initializeGui()
        # self.frameContainer = tk.Frame(self)
        # self.frameContainer.pack(fill="both",expand=True)
        self.frames = {}
        self.addFrames()

        self.setGridFrames()
        self.showLoginFrame()
        #self.connect_to_server()

    def addFrames(self):
        self.lobbyFrame = Lobby(root,self.client_socket)
        self.loginFrame = Login(root,self.lobbyFrame,self.client_socket)
        

    def setGridFrames(self):
        self.loginFrame.grid(row=0, column=0, sticky='news',pady=(0,100))
        self.lobbyFrame.grid(row=0, column=0, sticky='news',pady=(0,100))
    def initializeGui(self):
>>>>>>> main
        self.root.geometry('1000x600')
        self.root.resizable(0,0)


        self.frameManager = FrameManager(self.root,self.client_socket)
        self.frameManager.initFrames()
        self.frameManager.showFrame("Login")



if __name__ == "__main__":
    root = tk.Tk()
    app = QuizClient(root)
    root.mainloop()
