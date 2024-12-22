import tkinter as tk
from gui.login import Login
from gui.lobby import Lobby
from gui.gameRoom import GameRoom
class FrameManager():
    def __init__(self,root,socket):
        self.frames={}
        self.root=root
        self.states = {} # Do zrobienia
        self.socket=socket

    def initFrames(self):
        for FrameClass in (Lobby, Login, GameRoom):
            frame_name = FrameClass.__name__
            frame = FrameClass(self.root,self,self.socket)
            frame.grid(row=0, column=0, sticky='news',pady=(0,100))
            self.frames[frame_name] = frame


    def showFrame(self,frame):
        frame = self.frames[frame]
        frame.tkraise()




    
    

    
