#!/usr/bin/env python3
import socket
import struct

target_ip = "192.168.14.1"
handshake_pkt = bytes.fromhex("21310020ffffffffffffffffffffffffffffffffffffffffffffffffffffffff")

print(f"📡 Testando handshake UDP na Câmera {target_ip}:54321...")

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(3.0)

try:
    sock.sendto(handshake_pkt, (target_ip, 54321))
    data, addr = sock.recvfrom(1024)
    if data and len(data) >= 32:
        device_id = struct.unpack(">I", data[8:12])[0]
        stamp = struct.unpack(">I", data[12:16])[0]
        print(f"🎉 SUCESSO! Câmera respondeu no IP {addr[0]}!")
        print(f"   Device ID: {hex(device_id)} | Stamp: {stamp}")
    else:
        print("Aviso: Resposta curta ou inválida.")
except socket.timeout:
    print(f"❌ Timeout: A câmera {target_ip} não respondeu ao pacote UDP 54321.")
except Exception as e:
    print(f"Erro: {e}")
finally:
    sock.close()
