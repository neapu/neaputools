import socket
import time

# udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# addr = ('0.0.0.0', 5769)
udp_socket = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
addr = ('::', 5769)
udp_socket.bind(addr)


while 1:
    recv_data = udp_socket.recvfrom(1024)
    print(recv_data[0].decode(encoding='utf-8'))
    print(recv_data[1])
    udp_socket.sendto(recv_data[0], recv_data[1])