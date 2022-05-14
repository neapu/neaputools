import socket
import sys

def RunIPv4():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    msg = "test"
    addr = ("127.0.0.1", 8567)
    sock.sendto(msg.encode("utf-8"), addr)
    print("send message", msg)

def RunIPv6(address):
    sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)

    msg = "test"
    addr = (address, 8567)
    sock.sendto(msg.encode("utf-8"), addr)
    print("send message", msg)

if "__main__" == __name__:
    if len(sys.argv)<2:
        print("no address")
    else:
        RunIPv6(sys.argv[1])