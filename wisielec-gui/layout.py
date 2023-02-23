import PySimpleGUI as sg
import visuals

OTHER_HANGMEN_KEY = 'OTHER_HANGMEN'
YOUR_HANGMAN_KEY = 'YOUR_HANGMAN'
PHRASE_KEY = 'LBL_PHRASE'
CONFIRM_KEY = 'BTN_CONFIRM'
GUESS_KEY = 'TXT_GUESS'
SCORES_KEY = 'SCORES'

playerName = None

def createWindow(player):
    global playerName
    playerName = player

    scoreboard = sg.Table(
        [],
        ['Player name', 'Score', 'Fails'],
        key=SCORES_KEY
    )
    scoreboardColumn = sg.Column([[scoreboard]])

    otherHangmen = sg.Text(
        'Other\'s Hangmen\n' + 
        '  o  \n' +
        ' /|\\ \n' +
        ' / \\ \n',
        font='Courier 10',
        key=OTHER_HANGMEN_KEY
    )
    yourHangman = sg.Text(
        ' You \n' + 
        '  o  \n' +
        ' /|\\ \n' +
        ' / \\ \n',
        font='Courier 14',
        key=YOUR_HANGMAN_KEY
    )

    inputArea = sg.Column([
        [sg.Text('_', font='Courier 12', key=PHRASE_KEY)],
        [sg.InputText(s=5, key=GUESS_KEY), sg.Button('Try', key=CONFIRM_KEY)]
    ])

    gameColumn = sg.Column([
        [otherHangmen],
        [sg.HSep()],
        [yourHangman, sg.VSep(), inputArea]
    ])

    layout = [[
        scoreboardColumn,
        sg.VSep(),
        gameColumn
    ]]

    window = sg.Window(f'Hangman: {player}', layout, finalize=True)
    window.bind('<Return>', CONFIRM_KEY)
    return window

def clearGuessBox(window):
    window[GUESS_KEY].update('')

def updatePhrase(window, guesses):
    text = guesses['phrase']
    text = ' '.join(text)
    window[PHRASE_KEY].update(text)


scoreboard = dict()
def displayStoredScoreboard(window):
    global scoreboard
    scoresArray = []
    for player in scoreboard:
        scoresArray.append([player, scoreboard[player][0], scoreboard[player][1]])
    scoresArray.sort(key=lambda x: (x[1], -x[2]), reverse=True)
    window[SCORES_KEY].update(scoresArray)
    updateHangmen(window)

def displayScoreboard(window, scoreboardData):
    global scoreboard
    scoreboard = dict()
    for player in scoreboardData:
        scoreboard[player['name']] = [player['score'], player['fails']]
    displayStoredScoreboard(window)

def updateScore(window, score):
    global scoreboard
    if score['player'] not in scoreboard:
        return
    scoreboard[score['player']][0] = score['score']
    displayStoredScoreboard(window)

def updateFails(window, fails):
    global scoreboard
    if fails['player'] not in scoreboard:
        return
    scoreboard[fails['player']][1] = fails['fails']
    displayStoredScoreboard(window)

def updateHangmen(window):
    yourFails = scoreboard[playerName][1]
    window[YOUR_HANGMAN_KEY].update(' You \n' + '\n'.join(visuals.hangmenVariants[yourFails]))
    renderOthersHangmen(window)

def renderOthersHangmen(window):
    playerNames = list(scoreboard.keys())
    playerNames.remove(playerName)
    playerNames.sort()

    otherHangmenLines = ['', '', '', '']
    for player in playerNames:
        fails = scoreboard[player][1]

        nameLength = max(len(player), 5) + 2
        otherHangmenLines[0] += player.ljust(nameLength)
        for i in range(1, len(otherHangmenLines)):
            otherHangmenLines[i] += visuals.hangmenVariants[fails][i-1].ljust(nameLength)

    window[OTHER_HANGMEN_KEY].update('\n'.join(otherHangmenLines))

def addPlayer(window, player):
    global scoreboard
    scoreboard[player['name']] = [0, 0]
    displayStoredScoreboard(window)

def removePlayer(window, player):
    global scoreboard
    if player['name'] not in scoreboard:
        return
    del scoreboard[player['name']]
    displayStoredScoreboard(window)

def gameLost(window):
    window[PHRASE_KEY].update('YOU LOST!')
    window[CONFIRM_KEY].update(disabled=True)
    window[GUESS_KEY].update(disabled=True)

def gameWon(window):
    window[PHRASE_KEY].update('YOU WON!')
    window[CONFIRM_KEY].update(disabled=True)
    window[GUESS_KEY].update(disabled=True)

