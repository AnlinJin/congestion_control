#ifndef CONFIG_H
#define CONFIG_H

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/select.h>

#define PACKETLEN 1500
#define SENDBUFLEN 1484
#define CHUNKSIZE 20
#define MAX_ID 74

#define WHOHAS_PACK 0
#define IHAVE_PACK 1
#define GET_PACK 2
#define DATA_PACK 3
#define ACK_PACK 4
#define DENIED_PACK 5

#define PACK_HEADER_LEN 16 
#define VERSION 1
#define MAGIC_NUMBER 15441


typedef struct header_s
{
  short magicnum;
  char version;
  char packet_type;
  short header_len;
  short packet_len;
  __u_int seq_num;
  __u_int ack_num;
} header_t;

typedef struct data_packet
{
  header_t header;
  char data[SENDBUFLEN];
} data_packet_t;
#endif
