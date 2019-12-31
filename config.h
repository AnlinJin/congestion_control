
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