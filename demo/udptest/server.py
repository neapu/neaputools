import socket
udp_socket = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
addr = ('', 5769)
udp_socket.bind(addr)
recv_data = udp_socket.recvfrom(1024)
print(recv_data)