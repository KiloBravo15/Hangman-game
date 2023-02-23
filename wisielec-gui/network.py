import socket

sharedSocket = None

def connect(ipAddress, port):
    global sharedSocket
    sharedSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sharedSocket.connect((ipAddress, port))
    sharedSocket.setblocking(False)

def sendBytes(b):
    global sharedSocket
    print(f'Sending: {b!r}')
    sharedSocket.sendall(b)

def sendString(s):
    b = s.encode('utf-8')
    length = len(b)
    lengthBytes = length.to_bytes(4, byteorder='big')
    sendBytes(lengthBytes + b)

def receiveBytes(count):
    global sharedSocket
    try:
        return sharedSocket.recv(count)
    except socket.timeout:
        return None
    except BlockingIOError:
        return None

def receiveString():
    lengthBytes = b''
    while len(lengthBytes) < 4:
        res = receiveBytes(4 - len(lengthBytes))
        if res is None:
            return (None, None)
        if len(res) == 0:
            raise ConnectionResetError
        lengthBytes += res
    length = int.from_bytes(lengthBytes, byteorder='big')

    data = b''
    while len(data) < length:
        res = receiveBytes(length - len(data))
        if res is not None:
            data += res
        if len(res) == 0:
            raise ConnectionResetError
    return data[0], data[1:].decode('utf-8')

def disconnect():
    global sharedSocket
    sharedSocket.shutdown(socket.SHUT_RDWR)
    sharedSocket.close()
