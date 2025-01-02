import tkinter as tk
from gui.login import Login
from gui.lobby import Lobby
from gui.gameRoom import GameRoom
from gui.quizView import QuizView
class FrameManager():
    def __init__(self,root,socket):
        self.frames={}
        self.root=root
        self.states = {} # Do zrobienia
        self.socket=socket
        self.nick=None

    def initFrames(self):
        for FrameClass in (Lobby, Login, GameRoom,QuizView):
            frame_name = FrameClass.__name__
            frame = FrameClass(self.root,self,self.socket)
            frame.grid(row=0, column=0, sticky='news',pady=(0,100))
            self.frames[frame_name] = frame

    def setNickname(self,nick):
        self.nick = nick
    def getNick(self):
        if self.nick == None:
            pass
        else:
            return self.nick

    def showFrame(self,frame):
        frame = self.frames[frame]
        frame.tkraise()




    
    

    
