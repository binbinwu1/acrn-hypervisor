#!/bin/bash

# make sure there is enough 2M hugepages in the pool
# echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

# offline SOS CPUs except BSP before launch UOS
for i in `ls -d /sys/devices/system/cpu/cpu[1-99]`; do
        online=`cat $i/online`
        idx=`echo $i | tr -cd "[1-99]"`
        echo cpu$idx online=$online
        if [ "$online" = "1" ]; then
                echo 0 > $i/online
                echo $idx > /sys/class/vhm/acrn_vhm/offline_cpu
        fi
done

#acrn-dm -T -A -m 4096M -c 1 -s 0:0,hostbridge -s 1:0,lpc -l com1,stdio \
#  -s 5,virtio-console,@pty:pty_port \
#  --cafe /usr/share/acrn/cafe.bin cafe

acrn-dm -A -m 4096M -c 1 -s 0:0,hostbridge -s 1:0,lpc -l com1,stdio \
  --rit /usr/share/acrn/rit/rit.bin rit
