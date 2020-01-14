#ifndef WINDOW_H
#define WINDOW_H

#include <sys/types.h>

#include "config.h"
#include "dplist.h"
#include <stdlib.h>
#include <stdio.h>
#include "spiffy.h"
#include "debug.h"

#define sequen_t __u_int 
#define WINDOWSIZE 8
#define DATACHUNKSIZE 524288 //chunk size is 512kB
#define TIMEOUT   1 //timeout = 1s, for time() function, no timeout less than 1 second is possible
#define DUPLIMIT 3

typedef struct {
    unsigned char dupl_count;  //if duplicate count = 3, the packet is considered lost
    time_t lastSent;
}packet_info_t;
typedef struct {
    sequen_t LastAckReceived;
    sequen_t LastPacketSent;
    sequen_t LastPacketAvailable;
    unsigned char window_size;
    packet_info_t infos[WINDOWSIZE];
    data_packet_t packets[WINDOWSIZE];
    
    //int next_insert; //next array index to insert new packet 

}sending_window_t;

//This is a dummy receive window. It only contains some useful variable for convenience
typedef struct {
    long bytesReceived;
    sequen_t LastPacketReceived;
}receiver_window_t;




void send_data_packet(int sockfd,struct sockaddr_in *to_addr,socklen_t tolen);

//split a chunk file and generate multiple DATA packets. store these DATA packets into linked list 
//offset should start with 0
//filepath should be the path of masterdata file
void split_chunk_file(char *filepath, int offset);

//send a GET packet to the server
void send_get_packet(char *hash,int sockfd, struct sockaddr_in *to_addr,socklen_t tolen);
// receive the data from the server
void receive_data_packet(int sockfd,struct sockaddr_in *from,socklen_t fromlen, char *outputFile);

#endif