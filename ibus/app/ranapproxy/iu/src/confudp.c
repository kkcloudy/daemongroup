/****************************************************************************
** Description:
** Code to describe usage of M3UA API's.
*****************************************************************************
** Copyright(C) 2005 Shabd Communications Pvt. Ltd. http://www.shabdcom.org
*****************************************************************************
** Contact:
** vkgupta@shabdcom.org
*****************************************************************************
** License :
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*****************************************************************************/

#include <confasp1.h>
#include "sccp.h"
#include "common/list.h"

int num_packets_sent;
int cc_success;
#define ASP_STATE(state, p)	if (0 == state) p = "DOWN"; \
				else if (1 == state) p = "INACTIVE"; \
				else if (2 == state) p = "ACTIVE";
int main(int argc, char *argv[])
{
    if (3 > argc) {
        printf("Usage: %s asp=<IP Address>:<Port> "
               "sgp=<IP Address>:<Port>\n", argv[0]);
        exit(1);
    }
    /**** Initialise the Transport Layer ****/
    udp_transport_init(argv);
    
    waitfor_message(NULL);
    return 0;
}

int um3_get_asp_addr(char *argv[], udp_addr_t *paddr)
{
    char        *p;
    char        *ip;
    char        *port;

    if (0 != strncmp("asp", argv[1], 3))
        return -1;
    p = strchr(argv[1], '=');
    if (NULL == p)
        return -1;
    p++;
    if (0 == strlen(p))
        return -1;
    ip = strtok(p, ":");
    port = strtok(NULL, ":");
    paddr->ipaddr = inet_addr(ip);
    paddr->port = atoi(port);
printf("at asp.c asp paddr->ipaddr %x, port %x \n", paddr->ipaddr, paddr->port);
    return 0;
}

int um3_get_sgp_addr(char *argv[], udp_addr_t *paddr)
{
    char        *p;
    char        *ip;
    char        *port;

    if (0 != strncmp("sgp", argv[2], 3))
        return -1;
    p = strchr(argv[2], '=');
    if (NULL == p)
        return -1;
    p++;
    if (0 == strlen(p))
        return -1;
    ip = strtok(p, ":");
    port = strtok(NULL, ":");
    paddr->ipaddr = inet_addr(ip);
    paddr->port = atoi(port);
printf("at asp.c sgp paddr->ipaddr %x, port %x \n", paddr->ipaddr, paddr->port);
    return 0;
}

int udp_transport_init(char *argv[])
{
    udp_addr_t        addr;
    int               asp1_ep, sgp1_ep;

    /* create transport layer endpoints */
    um3_get_asp_addr(argv, &addr);
    asp1_ep = make_lep(addr);
    um3_get_sgp_addr(argv, &addr);
    sgp1_ep = make_rep(addr);

    /* create associations between the transport layer endpoints */
    make_assoc(asp1_ep, sgp1_ep, 0);
    return 0;
}

