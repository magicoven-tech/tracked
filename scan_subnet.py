#!/usr/bin/env python3
"""
Fast ARP & Port Scanner on 192.168.0.X to find new devices on the local router
"""
import socket
import concurrent.futures

def check_ip(ip):
    # Try connecting to any common device port
    for port in [54321, 80, 554, 8080, 443]:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(0.3)
        res = sock.connect_ex((ip, port))
        sock.close()
        if res == 0:
            return ip, port
    return None

def main():
    print("🔍 Varrendo todos os IPs da rede 192.168.0.1 - 192.168.0.254...")
    active_devices = []
    
    with concurrent.futures.ThreadPoolExecutor(max_workers=50) as executor:
        futures = [executor.submit(check_ip, f"192.168.0.{i}") for i in range(1, 255)]
        for future in concurrent.futures.as_completed(futures):
            res = future.result()
            if res:
                active_devices.append(res)
                print(f"  ✨ Dispositivo Ativo encontrado: {res[0]} (Porta TCP {res[1]})")

    print("\nResumo da rede 192.168.0.X:")
    for ip, port in active_devices:
        print(f"  👉 IP: {ip} | Porta: {port}")

if __name__ == "__main__":
    main()
