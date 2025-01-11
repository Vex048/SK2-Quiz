import socket
import threading
import tkinter as tk
from tkinter import messagebox
from gui.login import Login
from gui.lobby import Lobby
from gui.gameRoom import GameRoom
from gui.frameManager import FrameManager
import json
SERVER_IP = "172.18.43.116"
SERVER_PORT = 8080

class QuizClient:
    def __init__(self, root):
        self.root = root
        self.root.title("Quiz Game")
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #self.client_socket.connect((SERVER_IP, SERVER_PORT))
        self.root.geometry('1000x600')
        self.root.resizable(0,0)
        self.rootHeight=self.root.winfo_height()
        self.rootWidth=self.root.winfo_width()
        self.root.columnconfigure(0,weight=1)
        self.root.rowconfigure(0,weight=1)
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.stop_thread = False  
        


        self.frameManager = FrameManager(self.root,self.client_socket)
        self.frameManager.initLoginFrame()
        #self.frameManager.initFrames()
        self.frameManager.showFrame("Login")
        listenForServerUpdate = threading.Thread(target=self.listenForServerUpdates,args=(lambda: self.frameManager.stop_thread,))
        self.frameManager.getThread(listenForServerUpdate)



    def on_closing(self):
        self.stop_thread = True 
        self.frameManager.killThread()  
        try:
            answer_data = {
                "type": "quit"
            }
            jsonStr = json.dumps(answer_data) + "\n"
            self.client_socket.send(jsonStr.encode("utf-8"))
        except Exception as e:
            print(f"Error while sending quit message: {e}")
        finally:
            self.root.destroy()

    def listenForServerUpdates(self,stop):      
        buffer=""
        self.client_socket.settimeout(1.0) 
        while True:
            if stop():
                break
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
                            elif update["type"] == "new_question" or  update["type"] =="game_finished" or update["type"] == "answer_to_cur_question":
                                self.frameManager.frames["QuizView"].handle_update(update)
                            else:
                                self.frameManager.frames["Lobby"].handleUpdate(update)
                        except json.JSONDecodeError as e:
                            print(f"JSON decode error: {e}, received message: {json_str}")
                            break
            except socket.timeout:
                continue
            except Exception as e:
                print(f"Error: {e}")
                break    

if __name__ == "__main__":
    root = tk.Tk()
    app = QuizClient(root)
    root.mainloop()
