import socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
ipv4 = input("What is the AudioVisualiser ip handle?")
port = input("What is the AudioVisualiser port?")
client_socket.connect((ipv4, int(port)))
cue = input("Enter a cue.")
client_socket.send(cue.encode())
client_socket.close()