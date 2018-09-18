/*
 * udp_broadcast.h
 *
 *  Created on: Feb 4, 2018
 *      Author: heepdog
 */

#ifndef MAIN_UDP_BROADCAST_H_
#define MAIN_UDP_BROADCAST_H_

#define SENDER_PORT_NUM 6000
#define RECEIVER_PORT_NUM 6000
#define RECEIVER_IP_ADDR "255.255.255.255"

void task_UDP_broadcast(void *pvParams);

#endif /* MAIN_UDP_BROADCAST_H_ */
