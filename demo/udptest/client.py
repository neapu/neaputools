import socket
import sys

def RunIPv4(address, port, bindport):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    bind_addr = ('', int(bindport))
    sock.bind(bind_addr)
    msg = "test"
    addr = (address, int(port))
    sock.sendto(msg.encode("utf-8"), addr)
    print("send message", msg)
    recv_data = sock.recvfrom(1024)
    print(recv_data)

def RunIPv6(address, port, bindport):
    sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    bind_addr = ('', int(bindport))
    sock.bind(bind_addr)
    msg = "test"
    addr = (address, int(port))
    sock.sendto(msg.encode("utf-8"), addr)
    print("send message", msg)
    recv_data = sock.recvfrom(1024)
    print(recv_data)

if "__main__" == __name__:
    if len(sys.argv)<4:
        print('use: type addr connectport bindport')
        print('example: v4 127.0.0.1 9884 7886')
    elif sys.argv[1] == 'v6':
        RunIPv6(sys.argv[2], sys.argv[3], sys.argv[4])
    else:
        RunIPv4(sys.argv[2], sys.argv[3], sys.argv[4])
