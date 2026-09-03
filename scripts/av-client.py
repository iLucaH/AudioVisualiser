import socket

COMMAND_AUTH = 0
RESPONSE_ERR = "0"
RESPONSE_OK = "1"

def receive_response():
    response = client_socket.recv(1024)

    if not response:
        return None

    return response.decode().strip()


client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

ipv4 = input("What is the AudioVisualiser IP handle? ")
port = int(input("What is the AudioVisualiser port? "))

client_socket.connect((ipv4, port))

print("Connected to AudioVisualiser!")

password = input("Password: ")

auth_cue = f"{COMMAND_AUTH}:{password}"

print("Sending authentication...")
client_socket.sendall(auth_cue.encode())

# Wait for authentication response
response = receive_response()

if response is None:
    print("Server disconnected.")
    client_socket.close()
    exit()

print("Authentication response:", response)

if response != RESPONSE_OK:
    print("Authentication failed.")
    client_socket.close()
    exit()

print("Authenticated!")

print("Enter cues. Type 'exit' to disconnect.")

while True:
    cue = input("Cue: ")

    if cue.lower() == "exit":
        break

    # Send cue
    client_socket.sendall(cue.encode())

    # Wait for C++ response
    response = receive_response()

    if response is None:
        print("Server disconnected.")
        break

    print("Response:", response)

client_socket.close()
print("Disconnected.")