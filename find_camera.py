#!/usr/bin/env python3
"""
MagicTracked - Local Network Camera Scanner (Find Camera IP)
=============================================================
Escaneia a rede local (192.168.0.X ou 192.168.1.X) para encontrar a Câmera Xiaomi MJSXJ05CM
assim que ela se conectar à rede Wi-Fi Baia_2G.

Uso:
  python3 find_camera.py
"""

import socket
import struct
import select

def build_handshake():
    return bytes.fromhex("21310020ffffffffffffffffffffffffffffffffffffffffffffffffffffffff")

def scan_network():
    print("==================================================================")
    print(" 🎥 MagicTracked - Buscador de Câmera Xiaomi na Rede Local        ")
    print("==================================================================")
    print("🔍 Escaneando faixa de IPs da rede Wi-Fi local (porta UDP 54321)...")

    # Descobre subredes locais comuns
    subnets = ["192.168.0", "192.168.1", "192.168.15", "10.0.0"]
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(0.15)
    
    handshake = build_handshake()
    found_ip = None

    for sub in subnets:
        print(f"📡 Procurando na subrede {sub}.X ...")
        for i in range(1, 255):
            ip = f"{sub}.{i}"
            try:
                sock.sendto(handshake, (ip, 54321))
            except Exception:
                pass
        
        # Leitura de respostas
        sock.settimeout(1.0)
        try:
            while True:
                data, addr = sock.recvfrom(1024)
                if data and len(data) >= 32 and data[0:2] == bytes.fromhex("2131"):
                    dev_id = struct.unpack(">I", data[8:12])[0]
                    found_ip = addr[0]
                    print(f"\n✅ CÂMERA ENCONTRADA COM SUCESSO!")
                    print(f"👉 IP da Câmera na Rede: {found_ip}")
                    print(f"👉 Device ID: {hex(dev_id)}")
                    print(f"👉 URL do Stream RTSP: rtsp://{found_ip}:554/live/ch0")
                    return found_ip
        except socket.timeout:
            pass

    print("\n⚠️ Nenhuma câmera respondeu na rede local ainda.")
    print("Verifique se o Mac e a câmera estão conectados na mesma rede 'Baia_2G'.")
    return None

if __name__ == "__main__":
    scan_network()
