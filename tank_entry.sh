#!/bin/sh
cd /root/lab5
make
/root/lab5/tank_entry.exe &
/root/lab5/bt_listener.exe &

wait
echo all processes complete
