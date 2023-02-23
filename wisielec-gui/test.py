import network
import game_client

def printResponse(data):
    mtype = data[0]
    content = data[1]
    print(f"Received [0x{mtype:02x}]: {content}")

HOST = 'localhost'
PORT = 8080

network.connect(HOST, PORT)

game_client.join('')
data = None, None
while data[0] is None:
    data = game_client.getResponse()
printResponse(data)

game_client.join('A')
data = None, None
while data[0] is None:
    data = game_client.getResponse()
printResponse(data)

game_client.guess('H')
data = None, None
while data[0] is None:
    data = game_client.getResponse()
printResponse(data)

game_client.requestScoreboard()
data = None, None
while data[0] is None:
    data = game_client.getResponse()
printResponse(data)

network.disconnect()
