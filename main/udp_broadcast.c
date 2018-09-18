
#include <string.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>

#include "udp_broadcast.h"



void task_UDP_broadcast(void *pvParams){

    int socket_fd;
    struct sockaddr_in sa,ra;
    char* my_ip= (char *)pvParams;

    int sent_data; char data_buffer[80];
    /* Creates an UDP socket (SOCK_DGRAM) with Internet Protocol Family (PF_INET).
     * Protocol family and Address family related. For example PF_INET Protocol Family and AF_INET family are coupled.
    */
    socket_fd = socket(PF_INET, SOCK_DGRAM, 0);

    if ( socket_fd < 0 )
    {

        printf("socket call failed");
        exit(0);

    }

    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(my_ip);
    sa.sin_port = htons(SENDER_PORT_NUM);


    /* Bind the TCP socket to the port SENDER_PORT_NUM and to the current
    * machines IP address (Its defined by SENDER_IP_ADDR).
    * Once bind is successful for UDP sockets application can operate
    * on the socket descriptor for sending or receiving data.
    */
    if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == -1)
    {
      printf("Bind to Port Number %d ,IP address %s failed\n",SENDER_PORT_NUM,my_ip /*SENDER_IP_ADDR*/);
      close(socket_fd);
      exit(1);
    }
    printf("Bind to Port Number %d ,IP address %s SUCCESS!!!\n",SENDER_PORT_NUM,my_ip);



    memset(&ra, 0, sizeof(struct sockaddr_in));
    ra.sin_family = AF_INET;
    ra.sin_addr.s_addr = inet_addr(RECEIVER_IP_ADDR);
    ra.sin_port = htons(RECEIVER_PORT_NUM);


    strcpy(data_buffer,"Hello World\n");
    for (;;) {
        //#define sendto(s,dataptr,size,flags,to,tolen)     lwip_sendto_r(s,dataptr,size,flags,to,tolen)
        sent_data = sendto(socket_fd, data_buffer,sizeof("Hello World/n"),0,(struct sockaddr*)&ra,sizeof(ra));
        if(sent_data < 0)
        {
            printf("send failed\n");
            close(socket_fd);
            exit(2);
        }
        vTaskDelay(4000 / portTICK_PERIOD_MS);

    }
    close(socket_fd);
	vTaskDelete(NULL);
}
