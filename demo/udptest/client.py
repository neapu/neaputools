import socket
import sys

def RunIPv4():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    msg = "test"
    addr = ("127.0.0.1", 8567)
    sock.sendto(msg.encode("utf-8"), addr)
    print("send message", msg)

def RunIPv6(address, port):
    sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    bind_addr = ('', 5769)
    sock.bind(bind_addr)
    msg = "test"
    addr = (address, int(port))
    sock.sendto(msg.encode("utf-8"), addr)
    print("send message", msg)
    recv_data = sock.recvfrom(1024)
    print(recv_data)

if "__main__" == __name__:
    if len(sys.argv)<3:
        print("arg error")
    else:
        RunIPv6(sys.argv[1], sys.argv[2])