//The listen port of peer is initialized to the port stated in the file
//
#ifndef PEER_H
#define PEER_H

#include "config.h"
#include "window.h"

void response(int sock,data_packet_t *from_packet, struct sockaddr_in *from,socklen_t fromlen,int *ids);
void get_master_data(char * chunkfile, char *master_data);
#endif 