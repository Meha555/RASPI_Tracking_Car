import socket
import threading

def handle_client(s,addr):
    with s:
        print(addr,"connected.") 
        while True:
            data = s.recv(1234) # 从客户端接收数据
            if not data:
                break
            s.sendall(data) # 将接收到的数据原封不动地传回客户端

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listenfd: # with语法糖会自动调用listenfd.close()
    listenfd.bind(("192.168.43.58", 1234))
    listenfd.listen()

    while True:
        connfd,addr = listenfd.accept()
        t = threading.Thread(target = handle_client, args = (connfd, addr))
        t.start()
        