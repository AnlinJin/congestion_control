#include <sys/types.h>
#define sequen_t __u_int 
#define WINDOWSIZE 8
#define CHUNKSIZE 512 //chunk size is 512kB
#define timeout   1 //timeout = 1s
#include <config.h>

typedef struct {
    sequen_t LastAckReceived;
    sequen_t LastPacketSent;
    sequen_t LastPacketAvailable;
    unsigned char window_size;
    packet_info_t infos[WINDOWSIZE];
    data_packet_t packets[WINDOWSIZE];

}sending_window_t;

typedef struct {
    unsigned char dupl_count;  //if duplicate count = 3, the packet is considered lost
    time_t lastSent;

}packet_info_t;

void send_data_packet(int sockfd,data_packet_t );

//split a chunk file and generate multiple DATA packets. store these DATA packets into linked list 
void split_chunk_file(char *filepath, int offset);