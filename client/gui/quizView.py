import tkinter as tk

class QuizView(tk.Frame):
    def __init__(self,masterRoot,FrameManager,socket):
        super().__init__(masterRoot)
        self.socket=socket
        self.frameManager=FrameManager
        tk.Label(self, text="Quiz View", font=("Arial", 24)).pack(pady=20)

        self.question_label = tk.Label(self, text="", font=("Arial", 18), wraplength=500, justify="center")
        self.question_label.pack(pady=20)

        self.answer_buttons = []
        for i in range(4):
            button = tk.Button(self, text=f"Option {i+1}", font=("Arial", 14), command=lambda i=i: self.send_answer(i))
            button.pack(pady=10, fill="x")
            self.answer_buttons.append(button)

        tk.Button(self,text="Exit Room",command=self.frameManager.frames["GameRoom"].exitRoom).pack()
        self.current_question = None

    def update_question(self, question_data):
        self.current_question = question_data
        self.question_label.config(text=question_data["question"])
        for i, option in enumerate(question_data["options"]):
            self.answer_buttons[i].config(text=option, state="normal")

    def send_answer(self, selected_option):
        if self.current_question is None:
            messagebox.showerror("Error", "No question loaded!")
            return

        answer_data = {
            "type": "answer",
            "question_id": self.current_question["id"],
            "selected_option": selected_option
        }
        self.socket.send(json.dumps(answer_data).encode())
        for button in self.answer_buttons:
            button.config(state="disabled")

    def handle_update(self, update):
        if update["type"] == "new_question":
            self.update_question(update["data"])