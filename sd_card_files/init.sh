#!/bin/sh
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
