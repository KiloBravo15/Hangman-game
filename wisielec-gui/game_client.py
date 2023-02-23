import network

MTYPE_MASK = 0x3f
MTYPE_JOIN = 0x01
MTYPE_LEAVE = 0x02
MTYPE_GUESS = 0x11
MTYPE_SCORE = 0x12
MTYPE_HANG = 0x13
MTYPE_WIN = 0x21
MTYPE_LOSE = 0x22

MDIR_MASK = 0xc0
MDIR_REQUEST = 0x00
MDIR_RESPONSE = 0x40
MDIR_NOTIFY = 0x80


playerName = None

def join(name):
    global playerName
    message = chr(MDIR_REQUEST | MTYPE_JOIN) + name
    network.sendString(message)
    playerName = name

def leave():
    message = chr(MDIR_REQUEST | MTYPE_LEAVE)
    network.sendString(message)

def guess(letter):
    message = chr(MDIR_REQUEST | MTYPE_GUESS) + letter
    network.sendString(message)

def requestScoreboard():
    message = chr(MDIR_REQUEST | MTYPE_SCORE)
    network.sendString(message)

def requestPhrase():
    message = chr(MDIR_REQUEST | MTYPE_GUESS)
    network.sendString(message)

def getResponse():
    try:
        mtype, content = network.receiveString()
        if mtype is None:
            return (None, None)

        if (mtype & MDIR_MASK) not in [MDIR_RESPONSE, MDIR_NOTIFY]:
            print(f'Unexpected request: 0x{mtype:02x}')
            return (None, None)

        return (mtype, content)
    except ConnectionResetError as e:
        network.disconnect()
        raise e

