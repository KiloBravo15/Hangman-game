import PySimpleGUI as sg
import json
import layout
import network
import game_client
import sys

def processResponse(window, mtype, content):
    print(f"Received [0x{mtype:02x}]: {content}")

    content = json.loads(content)
    if mtype == game_client.MTYPE_GUESS | game_client.MDIR_NOTIFY:
        layout.updatePhrase(window, content)
    elif mtype == game_client.MTYPE_SCORE | game_client.MDIR_NOTIFY:
        layout.updateScore(window, content)
    elif mtype == game_client.MTYPE_HANG | game_client.MDIR_NOTIFY:
        layout.updateFails(window, content)
    elif mtype == game_client.MTYPE_SCORE | game_client.MDIR_RESPONSE:
        layout.displayScoreboard(window, content)
    elif mtype == game_client.MTYPE_JOIN | game_client.MDIR_NOTIFY:
        layout.addPlayer(window, content)
    elif mtype == game_client.MTYPE_LEAVE | game_client.MDIR_NOTIFY:
        layout.removePlayer(window, content)
    elif mtype == game_client.MTYPE_WIN | game_client.MDIR_NOTIFY:
        layout.gameWon(window)
    elif mtype == game_client.MTYPE_LOSE | game_client.MDIR_NOTIFY:
        layout.gameLost(window)


if len(sys.argv) < 4:
    print('Usage: python3 main.py <remote addr> <port> <player name>')
    sys.exit(1)

addr = sys.argv[1]
port = int(sys.argv[2])
name = sys.argv[3]

network.connect(addr, port)
game_client.join(name)

while True:
    mtype, content = game_client.getResponse()
    if mtype == game_client.MTYPE_JOIN | game_client.MDIR_RESPONSE:
        content = json.loads(content)
        if content['success']:
            print(f'Joined as {game_client.playerName}')
            game_client.requestScoreboard()
            game_client.requestPhrase()
        else:
            print(f'Failed to join as {game_client.playerName}')
            exit(1)
        break

window = layout.createWindow(game_client.playerName)

try:
    # The main event loop
    while True:
        event, values = window.read(timeout=10, timeout_key='TIMEOUT')
        if event == sg.WIN_CLOSED:
            break
        elif event == layout.CONFIRM_KEY:
            game_client.guess(values[layout.GUESS_KEY])
            layout.clearGuessBox(window)

        mtype, content = game_client.getResponse()
        if mtype is not None:
            processResponse(window, mtype, content)

    game_client.leave()
    network.disconnect()

except ConnectionResetError:
    print('Server disconnected')

window.close()
