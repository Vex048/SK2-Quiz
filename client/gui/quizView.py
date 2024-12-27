import tkinter as tk

class QuizView(tk.Frame):
    def __init__(self,masterRoot,FrameManager,socket):
        super().__init__(masterRoot)
        self.socket=socket
        self.frameManager=FrameManager
        tk.Label(self, text="Quiz View", font=("Arial", 24)).pack(pady=20)