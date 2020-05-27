/*
 * Empty C++ Application
 */

#include <stdio.h>

#include "xparameters.h"

#include <stdio.h>
#include "xparameters.h"
#include "platform_config.h"
#include "xgpio_l.h"
#include "xil_printf.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xil_types.h"
#include "xtime_l.h"
#include "xil_io.h"
#include "xpseudo_asm.h"
#include "xil_mmu.h"

#include "netif/xadapter.h"
#include "lwip/udp.h"
#include "lwip/err.h"
#include "lwip/init.h"
#include "sleep.h"
#include "UDPDev.h"


/* defined by each RAW mode application */

void platform_enable_interrupts();
void init_platform();
void print_app_header();
int start_application();
int transfer_data();



static struct netif server_netif;
struct  netif *echo_netif1;

void
print_ip(char *msg, struct ip_addr *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

void
print_ip_settings(struct ip_addr *ip, struct ip_addr *mask, struct ip_addr *gw)
{

	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}

int main()
{
	struct ip_addr ipaddr, netmask, gw, RemoteAddr;

	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] =
	{ 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	echo_netif1 = &server_netif;

	init_platform();

	/* initliaze IP addresses to be used */
	IP4_ADDR(&ipaddr,  192, 168,   1, 10);
	IP4_ADDR(&netmask, 255, 255, 255,  0);
	IP4_ADDR(&gw,      192, 168,   1,  1);
	IP4_ADDR(&RemoteAddr,  192, 168,   1, 100);

	print_ip_settings(&ipaddr, &netmask, &gw);

	lwip_init();


	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(echo_netif1, &ipaddr, &netmask,
						&gw, mac_ethernet_address,
						PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return -1;
	}
	netif_set_default(echo_netif1);

	/* Create a new DHCP client for this interface.
	 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	 * the predefined regular intervals after starting the client.
	 */
	/* dhcp_start(echo_netif); */

	/* now enable interrupts */
		platform_enable_interrupts();

	/* specify that the network if is up */
	netif_set_up(echo_netif1);

	UDPDev udpObj;
	/* start the application (web server, rxtest, txtest, etc..) */
	udpObj.start_application();

	/* receive and process packets */
	uint8_t array[4] = {0x31, 0x32, 0x33, 0x34};
	while (1) {

		udpObj.transfer_data(echo_netif1);
		udpObj.udp_send(array, &RemoteAddr, 7);
	}


	return 0;
}
