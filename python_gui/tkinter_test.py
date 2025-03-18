import tkinter as tk

root = tk.Tk()
root.title("Test Tkinter")
root.geometry("400x300")
root.config(bg="#131313")

# Create a frame with a visible border and different background color
frame = tk.Frame(root, bg="red", width=400, height=300, bd=2, relief="solid")
frame.pack(fill="both", expand=True)

# Create a label with contrasting colors
label = tk.Label(frame, text="Hello, Tkinter!", fg="white", bg="blue", font=("Arial", 20))
label.pack(padx=20, pady=20)

root.mainloop()
