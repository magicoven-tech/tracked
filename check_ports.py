#!/usr/bin/env python3
"""
Test open ports on camera IP 192.168.14.1
"""
import socket

camera_ip = "192.168.0.17"
ports = [80, 554, 1935, 8000, 8080, 8554, 8888, 54321]

print(f"🔍 Testando portas abertas no IP da câmera {camera_ip}...")

for port in ports:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(1.5)
    result = sock.connect_ex((camera_ip, port))
    if result == 0:
        print(f"  ✅ PORTA TCP {port} ABERTA!")
    else:
        print(f"  ❌ Porta TCP {port} fechada/filtrada")
    sock.close()
