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
        self.root.geometry('1000x600')
        self.root.resizable(0,0)


        self.frameManager = FrameManager(self.root,self.client_socket)
        self.frameManager.initFrames()
        self.frameManager.showFrame("Login")



if __name__ == "__main__":
    root = tk.Tk()
    app = QuizClient(root)
    root.mainloop()
