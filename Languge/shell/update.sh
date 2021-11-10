#!/bin/bash

user=`id -u`
[ "$user" != "0" ] && echo "Must be root/sudo" && exit

spl_file=/home/yinwg/tmp/download/spl-dvb-lpddr4-hynix-uart-2666-secure.bin
boot_file=/home/yinwg/tmp/download/boot_uart.pkg

server_ip="10.1.2.137"
ip_addr="10.1.1.48"
load_addr=0x6000000

expect -c "
    set timeout 300
    spawn minicom -D /dev/ttyUSB0;
    expect  {
        "*moment*" { send \"\01\"; send \"s\"; exp_continue }
        "*Upload*" { send \"jj\r\"; exp_continue }
        "*Directory*" { send \"\r\"; send \"$spl_file\r\";exp_continue }
        "*complete*" { send \"\r\"; }
    }
    expect {
        "*ymodem*" { send \"\01\"; send \"s\"; send \"j\r\"; exp_continue }
        "*Directory*" { send \"\r\"; send \"$boot_file\r\"; }
    }
    send \"\r\r\"
    expect {
        "disable*watchdog*success*" { send \"\r\r\r\"; send \"setenv\ serverip\ $server_ip\r\"; send \"setenv\ ipaddr\ $ip_addr\r\"; send \"tftp\ $load_addr\ disk.img\r\"; exp_continue } 
        "*transferred*" { send \"otawrite\ all\ $load_addr\ \\\$\{filesize\}\ emmc\r\"; }
    }
    send \"\01\"; send \"x\"; send \"\r\";
"
sleep 2

echo "

"

echo "==================================="
echo "download successed"
echo "==================================="

