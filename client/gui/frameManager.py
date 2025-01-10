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

    def initLoginFrame(self):
        frame_name = Login.__name__
        frame = Login(self.root,self,self.socket)
        frame.pack(fill="both", expand=True)
        frame.pack_forget()
        self.frames[frame_name] = frame

    def initFrames(self):
        for FrameClass in (Lobby, GameRoom,QuizView):
            frame_name = FrameClass.__name__
            frame = FrameClass(self.root,self,self.socket)
            #frame.grid(row=0, column=0, sticky='news',pady=(0,100))
            frame.pack(fill="both", expand=True)
            frame.pack_forget()
            self.frames[frame_name] = frame

    def setNickname(self,nick):
        self.nick = nick
    def getNick(self):
        if self.nick == None:
            pass
        else:
            return self.nick
    def getThread(self,listen_thread):
        self.listen_thread=listen_thread
    def startListening(self):
        self.listen_thread.start()

    def showFrame(self,frame):
        #frame = self.frames[frame]
        # frame.tkraise()
        for name, frame1 in self.frames.items():
            if name == frame:
                frame1.pack(fill="both", expand=True)  
            else:
                frame1.pack_forget()




    
    

    
