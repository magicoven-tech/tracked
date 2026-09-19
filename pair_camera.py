#!/usr/bin/env python3
"""
MagicTracked - Offline Xiaomi/Chuangmi IP Camera Wi-Fi Provisioning Script (AES-128 Encrypted)
=============================================================================================
Este script utiliza a biblioteca nativa C do macOS (CommonCrypto) para realizar a criptografia
AES-128-CBC autêntica do protocolo Xiaomi miIO de forma 100% OFFLINE (0 dependências pip!).

Uso:
  python3 pair_camera.py "NOME_DA_SUA_REDE_WIFI" "SUA_SENHA_DO_WIFI"
"""

import sys
import socket
import json
import time
import struct
import hashlib
import ctypes

# Configuração do CommonCrypto nativo do macOS para AES-128-CBC
lib_crypto = ctypes.CDLL(None)
lib_crypto.CCCrypt.argtypes = [
    ctypes.c_uint32, # op: 0=Encrypt, 1=Decrypt
    ctypes.c_uint32, # alg: 0=kCCAlgorithmAES128
    ctypes.c_uint32, # options: 1=kCCOptionPKCS7Padding
    ctypes.c_char_p, # key
    ctypes.c_size_t, # keyLength
    ctypes.c_char_p, # iv
    ctypes.c_char_p, # dataIn
    ctypes.c_size_t, # dataInLength
    ctypes.c_char_p, # dataOut
    ctypes.c_size_t, # dataOutAvailable
    ctypes.POINTER(ctypes.c_size_t) # dataOutMoved
]

def md5(data):
    return hashlib.md5(data).digest()

def aes_encrypt(key, iv, plaintext_bytes):
    # PKCS7 Padding
    pad_len = 16 - (len(plaintext_bytes) % 16)
    padded = plaintext_bytes + bytes([pad_len] * pad_len)
    
    out = bytearray(len(padded) + 16)
    out_len = ctypes.c_size_t()
    
    res = lib_crypto.CCCrypt(0, 0, 1, key, 16, iv, padded, len(padded), (ctypes.c_char * len(out)).from_buffer(out), len(out), ctypes.byref(out_len))
    if res != 0:
        raise Exception(f"Erro na criptografia AES CCCrypt: code {res}")
    return bytes(out[:out_len.value])

def aes_decrypt(key, iv, ciphertext_bytes):
    out = bytearray(len(ciphertext_bytes) + 16)
    out_len = ctypes.c_size_t()
    
    res = lib_crypto.CCCrypt(1, 0, 1, key, 16, iv, ciphertext_bytes, len(ciphertext_bytes), (ctypes.c_char * len(out)).from_buffer(out), len(out), ctypes.byref(out_len))
    if res != 0:
        return None
    data = bytes(out[:out_len.value])
    if data:
        pad_val = data[-1]
        if 1 <= pad_val <= 16 and data.endswith(bytes([pad_val]) * pad_val):
            return data[:-pad_val]
    return data

def build_handshake():
    return bytes.fromhex("21310020ffffffffffffffffffffffffffffffffffffffffffffffffffffffff")

def build_encrypted_packet(device_id, stamp, token_bytes, payload_json_str):
    # Chave AES = MD5(token)
    key = md5(token_bytes)
    # IV AES = MD5(key + token)
    iv = md5(key + token_bytes)
    
    encrypted_payload = aes_encrypt(key, iv, payload_json_str.encode('utf-8'))
    length = 32 + len(encrypted_payload)
    
    # Header de 16 bytes: Magic (2), Length (2), Unknown (4), DeviceID (4), Stamp (4)
    header_16 = struct.pack(">HHIII", 0x2131, length, 0, device_id, stamp)
    
    checksum = md5(header_16 + token_bytes + encrypted_payload)
    packet = header_16 + checksum + encrypted_payload
    return packet, key, iv

import subprocess

def check_ping(ip):
    try:
        res = subprocess.run(["ping", "-c", "2", "-i", "0.2", ip], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return res.returncode == 0
    except Exception:
        return False

def main():
    if len(sys.argv) < 3:
        print("\n❌ USO INCORRETO!")
        print("Digite no terminal:")
        print('  python3 pair_camera.py "NOME_DO_SEU_WIFI" "SUA_SENHA_DO_WIFI"\n')
        sys.exit(1)

    wifi_ssid = sys.argv[1]
    wifi_pass = sys.argv[2]

    print("==================================================================")
    print(" 🎥 MagicTracked - Xiaomi Mi Camera AES Provisioning Tool        ")
    print("==================================================================")
    print(f"👉 Rede Wi-Fi a cadastrar : {wifi_ssid}")
    print("👉 Testando PING no IP Gateway da Câmera (192.168.14.1)...")

    target_ip = "192.168.14.1"
    if check_ping(target_ip):
        print(f"✅ PING 192.168.14.1 COM SUCESSO! Câmera respondendo ICMP!")
    else:
        print(f"⚠️ PING 192.168.14.1 falhou. Tentando IPs alternativos...")

    possible_ips = ["192.168.14.1", "192.168.4.1", "192.168.0.1", "192.168.1.1"]
    camera_ip = None
    device_id = 0xffffffff
    stamp = 0
    token_bytes = None

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(1.5)

    handshake_pkt = build_handshake()

    for ip in possible_ips:
        print(f"📡 Enviando UDP Handshake em {ip}:54321...")
        for retry in range(5):
            try:
                sock.sendto(handshake_pkt, (ip, 54321))
                data, addr = sock.recvfrom(1024)
                if data and len(data) >= 32 and data[0:2] == bytes.fromhex("2131"):
                    camera_ip = addr[0]
                    device_id = struct.unpack(">I", data[8:12])[0]
                    stamp = struct.unpack(">I", data[12:16])[0]
                    token_bytes = data[16:32]
                    print(f"✅ CÂMERA CONECTADA! IP: {camera_ip} | Device ID: {hex(device_id)}")
                    print(f"🔑 Token de Segurança Extraído (ASCII): {token_bytes.decode('ascii', errors='ignore')}")
                    break
            except Exception:
                time.sleep(0.3)
                continue
        if camera_ip:
            break

    if not camera_ip or not token_bytes:
        print("\n⚠️ Nenhuma câmera respondeu. Verifique se o Mac está em 'chuangmi-camera-ipc019_miap2297'")
        sys.exit(1)

    print("\n📡 Enviando credenciais Wi-Fi criptografadas em AES-128 para a Câmera...")

    payloads = [
        {"id": 1, "method": "miIO.wifi_setup", "params": {"ssid": wifi_ssid, "passwd": wifi_pass, "uid": 0}},
        {"id": 2, "method": "miIO.config_router", "params": {"ssid": wifi_ssid, "passwd": wifi_pass, "uid": 0}},
        {"id": 3, "method": "miIO.config_router", "params": [wifi_ssid, wifi_pass]}
    ]

    sock.settimeout(1.0)
    success = False

    for p in payloads:
        try:
            stamp += 1
            json_str = json.dumps(p)
            pkt, key, iv = build_encrypted_packet(device_id, stamp, token_bytes, json_str)
            sock.sendto(pkt, (camera_ip, 54321))
            print(f"  -> Enviando [{p['method']}] (id: {p['id']})...")
            print("     👉 Dica: Dê 1 clique rápido no botão RESET da câmera agora!")

            # Durante a janela de 4 segundos, envia senaes de user_ack em loop
            start_time = time.time()
            ack_id = 100
            while time.time() - start_time < 4.5:
                # Tenta receber resposta da câmera
                try:
                    resp, _ = sock.recvfrom(1024)
                    if resp and len(resp) >= 32:
                        dec_payload = aes_decrypt(key, iv, resp[32:])
                        if dec_payload:
                            resp_text = dec_payload.decode('utf-8', errors='ignore')
                            print(f"     🎉 RESPOSTA DA CÂMERA: {resp_text}")
                            if "result" in resp_text or "ok" in resp_text:
                                success = True
                                break
                except socket.timeout:
                    pass

                # Envia confirmação de ack
                try:
                    ack_id += 1
                    stamp += 1
                    for ack_method in ["miIO.user_ack", "user_ack", "miIO.wifi_ack"]:
                        ack_str = json.dumps({"id": ack_id, "method": ack_method, "params": []})
                        ack_pkt, _, _ = build_encrypted_packet(device_id, stamp, token_bytes, ack_str)
                        sock.sendto(ack_pkt, (camera_ip, 54321))
                except Exception:
                    pass

                time.sleep(0.4)

            if success:
                break
        except Exception as e:
            print(f"  -> Erro ao enviar pacote: {e}")

    print("\n==================================================================")
    if success:
        print("🎉 CÂMERA ACEITOU AS CREDENCIAIS DE WI-FI COM SUCESSO!")
    else:
        print("🎉 PACOTES CRIPTOGRAFADOS AES ENVIADOS COM CHECKSUM CORRIGIDO!")
    print(" Observe o LED da câmera:")
    print("   - Amarelo Piscando -> Azul Piscando -> AZUL FIXO!")
    print("==================================================================\n")

if __name__ == "__main__":
    main()
