import ttk
import tkMessageBox
from Tkinter import *

def connect():
	if server.get() == "":
		tkMessageBox.showerror(title = "Error", message = "No server!")
		return
	if user.get() == "":
		tkMessageBox.showerror(title = "Error", message = "No user!")
		return
	if password.get() == "":
		tkMessageBox.showerror(title = "Error", message = "No password!")
		return
	print("connect to %s as %s:%s" % (server.get(), user.get(), password.get()))

root = Tk()
root.title("Wendy Login")
root.resizable(False, False)

frame = ttk.Frame(root)
frame.grid()
frame.columnconfigure(0, pad = 4)
frame.columnconfigure(1, pad = 4, minsize = 200)
frame.rowconfigure(0, pad = 4)
frame.rowconfigure(1, pad = 4)
frame.rowconfigure(2, pad = 4)
frame.rowconfigure(3, pad = 4)

label = ttk.Label(frame, text = "Server")
label.grid(row = 0, column = 0)

server = StringVar()
serverEntry = ttk.Entry(frame, textvariable = server)
serverEntry.grid(row = 0, column = 1, sticky = "ew", padx = 4)

label2 = ttk.Label(frame, text = "User")
label2.grid(row = 1, column = 0)

user = StringVar()
userEntry = ttk.Entry(frame, textvariable = user)
userEntry.grid(row = 1, column = 1, sticky = "ew", padx = 4)

label3 = ttk.Label(frame, text = "Password")
label3.grid(row = 2, column = 0)

password = StringVar()
passwordEntry = ttk.Entry(frame, textvariable = password, show = "*")
passwordEntry.grid(row = 2, column = 1, sticky = "ew", padx = 4)

button = ttk.Button(text = "Connect", command = connect, default = "active")
button.grid(row = 3, column = 0)

root.mainloop()
