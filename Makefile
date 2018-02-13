CC=gcc
CFLAGS=-std=c99

all: adc_listener tank tank_entry bt_listener

adc_listener: adc_listener.c adc_listener.h lab5_common.c lab5_common.h
	$(CC) $(CFLAGS) -o adc_listener.exe adc_listener.c lab5_common.c -lrt

tank: tank.c tank.h lab5_common.c lab5_common.h
	$(CC) $(CFLAGS) -o tank.exe tank.c lab5_common.c

tank_entry: tank_entry.c tank_entry.h lab5_common.c lab5_common.h
	$(CC) $(CFLAGS) -o tank_entry.exe tank_entry.c lab5_common.c

bt_listener: bt_listener.c bt_listener.h 
	$(CC) $(CFLAGS) -o bt_listener.exe bt_listener.c 

clean:
	rm -rf *.exe
