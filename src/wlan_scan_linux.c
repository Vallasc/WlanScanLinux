
#include "wifi_scan.h"
#include <stdio.h> 
#include <unistd.h> 
#include <stdbool.h>
#include <string.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#pragma GCC diagnostic ignored "-Wstringop-truncation"

//convert bssid to printable hardware mac address
const char *bssid_to_string(const uint8_t bssid[BSSID_LENGTH], char bssid_string[BSSID_STRING_LENGTH])
{
	snprintf(bssid_string, BSSID_STRING_LENGTH, "%02x:%02x:%02x:%02x:%02x:%02x",
         bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
	return bssid_string;
}

const int BSS_INFOS=100; //the maximum amounts of APs (Access Points) we want to store

void show_help();
int check_wireless(const char* ifname, char* protocol);
void list_aps(char* interface_name, bool show_json);

int main(int argc, char **argv) {
	bool show_json = false;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
			show_help();
			return 0;
		} else if (strcmp("--json", argv[i]) == 0 || strcmp("-j", argv[i]) == 0) {
			show_json = true;
		} else {
			printf("Unrecognized command %s. Run --help.\n", argv[i]);
			return 0;
		}
	}


	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return -1;
	}

	/* Walk through linked list, maintaining head pointer so we
		can free list later */
	int i_count = 1;

	if(show_json)
		printf("[");
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		char protocol[IFNAMSIZ]  = {0};

		if (ifa->ifa_addr == NULL ||
			ifa->ifa_addr->sa_family != AF_PACKET) continue;

		// Check if it is a wireless interface
		if (check_wireless(ifa->ifa_name, protocol)) {
			if(!show_json){
				printf("\nInterface %d: %s\n\n", i_count++, ifa->ifa_name);
				printf("\t%-30s %-24s %-14s\n","SSID", "BSSID", "RSSI");
				printf("\t%-30s %-24s %-14s\n","---------------------------", "------------------", "-----");
			} else if (i_count > 1) {
				printf(",");
				i_count++;
			}
			list_aps(ifa->ifa_name, show_json);
			if(!show_json)
				printf("\n");
		}
	}
	if(show_json)
		printf("]");

	freeifaddrs(ifaddr);

	return 0;
}

void list_aps(char* interface_name, bool show_json)
{
	struct wifi_scan *wifi=NULL;    //this stores all the library information
	struct bss_info bss[BSS_INFOS]; //this is where we are going to keep informatoin about APs (Access Points)
	char mac[BSSID_STRING_LENGTH];  //a placeholder where we convert BSSID to printable hardware mac address
	int status, i;

	wifi=wifi_scan_init(interface_name);

	status=wifi_scan_all(wifi, bss, BSS_INFOS);

	//it may happen that device is unreachable (e.g. the device works in such way that it doesn't respond while scanning)
	//you may test for errno==EBUSY here and make a retry after a while, this is how my hardware works for example
	if(status<0 && !show_json)
		perror("Unable to get scan data");

	else //wifi_scan_all returns the number of found stations, it may be greater than BSS_INFOS that's why we test for both in the loop
		for(i=0;i<status && i<BSS_INFOS;++i)	
			if(!show_json) {
				printf("\t%-30s %-24s %-14d\n", bss[i].ssid, bssid_to_string(bss[i].bssid, mac), bss[i].signal_mbm/100);
			} else {
				if (i > 0)
					printf(",");
				printf("{\"BSSID\":\"%s\",\"SSID\":\"%s\",\"frequency\":%u,\"signal\":%d, \"interfaceName\":\"%s\"}",
					bssid_to_string(bss[i].bssid, mac),
					bss[i].ssid,
					bss[i].frequency,
					bss[i].signal_mbm/100,
					interface_name);
			}
	
	//free the library resources
	wifi_scan_close(wifi);

}

void show_help()
{
	printf("WlanScan - A small utility for triggering scans for wireless networks for Windows systems\n");
	printf("\t--json, -j\tShows networks in json format.\n");
	printf("\t--help, -h\tShows this help.\n");
}

int check_wireless(const char* ifname, char* protocol) 
{
  int sock = -1;
  struct iwreq pwrq;
  memset(&pwrq, 0, sizeof(pwrq));
  strncpy(pwrq.ifr_name, ifname, IFNAMSIZ);

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return 0;
  }

  if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1) {
    if (protocol) strncpy(protocol, pwrq.u.name, IFNAMSIZ);
    close(sock);
    return 1;
  }

  close(sock);
  return 0;
}