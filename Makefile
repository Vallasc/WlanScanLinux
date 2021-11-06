WIFI_SCAN = wifi_scan.o
CC = gcc
CXX = g++
DEBUG =
CFLAGS = -static -static-libgcc -O2 -Wall -c $(DEBUG)
CXX_FLAGS = -O2 -std=c++11 -Wall -c $(DEBUG)
LDLIBS = lib/libmnl.a  # static linking
#LDLIBS = -lmnl
OUT_DIR = bin
SRC_DIR = src
MKDIR_P = mkdir -p

all: directories wlan_scan_linux

directories: ${OUT_DIR}

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}

wlan_scan_linux : wifi_scan.o wlan_scan_linux.o
	$(CC) wifi_scan.o wlan_scan_linux.o $(LDLIBS) -o ${OUT_DIR}/WlanScanLinux

wifi_scan.o : $(SRC_DIR)/wifi_scan.h $(SRC_DIR)/wifi_scan.c
	$(CC) $(CFLAGS) $(SRC_DIR)/wifi_scan.c

wlan_scan_linux.o : $(SRC_DIR)/wifi_scan.h $(SRC_DIR)/wlan_scan_linux.c
	$(CC) $(CFLAGS) $(SRC_DIR)/wlan_scan_linux.c

clean:
	\rm -f *.o $(OUT_DIR)
