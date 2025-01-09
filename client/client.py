import socket
import threading
import tkinter as tk
from tkinter import messagebox
from gui.login import Login
from gui.lobby import Lobby
from gui.gameRoom import GameRoom
from gui.frameManager import FrameManager
import json
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
        self.rootHeight=self.root.winfo_height()
        self.rootWidth=self.root.winfo_width()
        self.root.columnconfigure(0,weight=1)
        self.root.rowconfigure(0,weight=1)


        self.frameManager = FrameManager(self.root,self.client_socket)
        self.frameManager.initFrames()
        self.frameManager.showFrame("Login")
        listenForServerUpdate = threading.Thread(target=self.listenForServerUpdates)
        listenForServerUpdate.start()



    def listenForServerUpdates(self):
        buffer=""
        while True:
            try:
                message = self.client_socket.recv(1024).decode()
                if not message:  
                    print("Connection closed by the server")
                    break
                buffer += message
                while "\n" in buffer:
                    json_str, buffer = buffer.split("\n", 1)
                    if json_str.strip():
                        print(f"Processing message: {json_str}")
                        try:
                            update = json.loads(json_str)
                            if update["type"] == "create_nickname":
                                  self.frameManager.frames["Login"].handleUpdateNick(update)
                            elif update["type"] == "new_question" or  update["type"] =="game_finished":
                                self.frameManager.frames["QuizView"].handle_update(update)
                            else:
                                self.frameManager.frames["Lobby"].handleUpdate(update)
                        except json.JSONDecodeError as e:
                            print(f"JSON decode error: {e}, received message: {json_str}")
                            break
            except Exception as e:
                print(f"Error: {e}")
                break    

if __name__ == "__main__":
    root = tk.Tk()
    app = QuizClient(root)
    root.mainloop()
