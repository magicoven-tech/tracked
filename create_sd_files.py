#!/usr/bin/env python3
"""
MagicTracked - 100% Comprehensive SD Card Pack for Xiaomi MJSXJ05CM
===================================================================
Cria TANTO os arquivos soltos quanto as pastas exigidas pelo firmware Xiaomi
(`manu_test/` e `hacks/`) na raiz do cartão MicroSD.

Uso:
  python3 create_sd_files.py "Baia_2G" "Baia246810"
"""

import sys
import os
import shutil

def main():
    wifi_ssid = sys.argv[1] if len(sys.argv) > 1 else "Baia_2G"
    wifi_pass = sys.argv[2] if len(sys.argv) > 2 else "Baia246810"

    out_dir = os.path.abspath("sd_card_files")
    if os.path.exists(out_dir):
        shutil.rmtree(out_dir)
    os.makedirs(out_dir, exist_ok=True)

    print("==================================================================")
    print(" 🎥 MagicTracked - Gerador de Arquivos MicroSD Completo (MJSXJ05CM)")
    print("==================================================================")
    print(f"👉 Rede Wi-Fi: {wifi_ssid}\n")

    # Conteúdo do wpa_supplicant.conf
    wpa_content = f"""ctrl_interface=/var/run/wpa_supplicant
update_config=1

network={{
    ssid="{wifi_ssid}"
    scan_ssid=1
    psk="{wifi_pass}"
    key_mgmt=WPA-PSK
}}
"""

    # Conteúdo do script shell de inicialização
    script_content = f"""#!/bin/sh
# Boot Script para Xiaomi MJSXJ05CM
export PATH=/bin:/sbin:/usr/bin:/usr/sbin

mkdir -p /tmp/wifi /etc/wpa_supplicant /data/etc
cp -f /mnt/sdcard/wpa_supplicant.conf /tmp/wifi/wpa_supplicant.conf 2>/dev/null
cp -f /mnt/sdcard/manu_test/wpa_supplicant.conf /tmp/wifi/wpa_supplicant.conf 2>/dev/null
cp -f /mnt/sdcard/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant.conf 2>/dev/null
cp -f /mnt/sdcard/wpa_supplicant.conf /data/etc/wpa_supplicant.conf 2>/dev/null

killall -9 wpa_supplicant 2>/dev/null
wpa_supplicant -B -i wlan0 -c /tmp/wifi/wpa_supplicant.conf
udhcpc -i wlan0 -b -s /etc/udhcpc.script &

telnetd -l /bin/sh &
"""

    # 1. Salva na raiz da pasta de saída
    with open(os.path.join(out_dir, "wpa_supplicant.conf"), "w", newline="\n", encoding="utf-8") as f:
        f.write(wpa_content)

    for fn in ["manu_test", "manu_test.sh", "system.sh", "init.sh", "factory_test.sh"]:
        fp = os.path.join(out_dir, fn)
        with open(fp, "w", newline="\n", encoding="utf-8") as f:
            f.write(script_content)
        os.chmod(fp, 0o755)

    # 2. Pasta manu_test (Padrão Oficial MJSXJ05CM Hacks)
    manu_folder = os.path.join(out_dir, "manu_test")
    # se manu_test já foi criado como arquivo solto, criamos pasta manu_test_dir e renomeamos para cobrir ambos
    # Na raiz: os arquivos soltos continuam existindo
    # E uma pasta manu_test/ é criada com manu_test dentro
    os.remove(os.path.join(out_dir, "manu_test"))
    os.makedirs(manu_folder, exist_ok=True)
    
    with open(os.path.join(manu_folder, "manu_test"), "w", newline="\n", encoding="utf-8") as f:
        f.write(script_content)
    os.chmod(os.path.join(manu_folder, "manu_test"), 0o755)

    with open(os.path.join(manu_folder, "manu_test.sh"), "w", newline="\n", encoding="utf-8") as f:
        f.write(script_content)
    os.chmod(os.path.join(manu_folder, "manu_test.sh"), 0o755)

    with open(os.path.join(manu_folder, "wpa_supplicant.conf"), "w", newline="\n", encoding="utf-8") as f:
        f.write(wpa_content)

    # 3. Pasta hacks/
    hacks_folder = os.path.join(out_dir, "hacks")
    os.makedirs(hacks_folder, exist_ok=True)
    with open(os.path.join(hacks_folder, "hacks.sh"), "w", newline="\n", encoding="utf-8") as f:
        f.write(script_content)
    os.chmod(os.path.join(hacks_folder, "hacks.sh"), 0o755)

    print("  ✅ Estrutura criada com sucesso em sd_card_files/:")
    print("     - wpa_supplicant.conf (arquivo solto na raiz)")
    print("     - manu_test.sh, system.sh, init.sh (arquivos soltos na raiz)")
    print("     - manu_test/ (pasta contendo manu_test e wpa_supplicant.conf)")
    print("     - hacks/ (pasta contendo hacks.sh)\n")

if __name__ == "__main__":
    main()
