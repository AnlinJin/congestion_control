/*
 * peer.c
 *
 * Authors: Ed Bardsley <ebardsle+441@andrew.cmu.edu>,
 *          Dave Andersen
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2.
 *
 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "spiffy.h"
#include "bt_parse.h"
#include "input_buffer.h"
#include <sys/select.h>
#include "peer.h"

#define RECVBUFLEN 1500

void peer_run(bt_config_t *config);

bt_config_t config;

int main(int argc, char **argv)
{

  bt_init(&config, argc, argv);

  DPRINTF(DEBUG_INIT, "peer.c main beginning\n");

#ifdef TESTING
  config.identity = 1; // your group number here
  strcpy(config.chunk_file, "chunkfile");
  strcpy(config.has_chunk_file, "haschunks");
#endif

  bt_parse_command_line(&config);

#ifdef DEBUG
  if (debug & DEBUG_INIT)
  {
    bt_dump_config(&config);
  }
#endif

  peer_run(&config);
  return 0;
}
void response(int sock, data_packet_t *from_packet, struct sockaddr_in *from, socklen_t fromlen, int *ids)
{
  FILE *has_chunk = fopen(config.has_chunk_file, "r");
  data_packet_t *ihave_packet = malloc(sizeof(data_packet_t));
  memset(ihave_packet, 0, RECVBUFLEN);
  ihave_packet->header.magicnum = 15441;
  ihave_packet->header.version = 1;
  ihave_packet->header.packet_type = 1;
  ihave_packet->header.header_len = 16;
  ihave_packet->header.seq_num = 0;
  ihave_packet->header.ack_num = 0;
  int nOfchunks = from_packet->data[0];
  int id;
  int nOfChunksHad = 0;
  int start_ihave = 4;
  int data_len = 20;
  unsigned char hash[CHUNKSIZE];
  unsigned char buffer[41]; //20 bytes hash string(each hex number is treated as a char)
  unsigned char v;
  //char current_hash[CHUNKSIZE];
  int start = 4;
  for (int i = 0; i < nOfchunks; i++)
  {
    while (fscanf(has_chunk, "%d %s", &id, buffer) == 2)
    {
      for (int j = 0; j < 20; j++)
      {
        if (sscanf((char *)(buffer) + 2 * j, "%2hhx", &v) != 1)
          break;
        hash[j] = v;
      }
      if (memcmp(hash, from_packet->data + start, CHUNKSIZE) == 0)
      {
        nOfChunksHad++;
        data_len += CHUNKSIZE;
        memcpy(ihave_packet->data + start_ihave, hash, CHUNKSIZE);
        ids[i] = id;
        start_ihave += CHUNKSIZE;
        break;
      }
    }
    start += 20;
  }

  if (nOfChunksHad != 0)
  {
    ihave_packet->data[0] = nOfChunksHad;
    for (int i = 1; i <= 3; i++)
    {
      ihave_packet->data[i] = 0;
    }
    ihave_packet->header.packet_len = data_len;
    spiffy_sendto(sock, (char *)ihave_packet, sizeof(data_packet_t), 0, (struct sockaddr *)from, fromlen);
  }

  free(ihave_packet);
}
void process_inbound_udp(int sock)
{

  struct sockaddr_in from;
  socklen_t fromlen = sizeof(from);
  data_packet_t *from_packet = malloc(sizeof(data_packet_t));
  memset(from_packet, 0, RECVBUFLEN);
  int readStatus;
  int ids[MAX_ID];
  memset(ids, 0, sizeof(ids));
  readStatus = spiffy_recvfrom(sock, from_packet, RECVBUFLEN, 0, (struct sockaddr *)&from, &fromlen);
  printf("WHOHAS packet received\n"
         "Incoming message from %s:%d\n%s\n\n",
         inet_ntoa(from.sin_addr),
         ntohs(from.sin_port), (char *)from_packet);

  if (readStatus < 0)
  {
    perror("reading error...\n");
    close(sock);
    exit(-1);
  }
  //sscanf(buf, "%hu%hhu%hhu%hu%hu%u%u%s", &from_header.magicnum, &from_header.version, &from_header.packet_type,
  // &from_header.header_len, &from_header.packet_len, &from_header.seq_num, &from_header.ack_num, from_packet->data);
  if (from_packet->header.magicnum == 15441 && from_packet->header.version == 1 && from_packet->header.packet_type == WHOHAS_PACK)
  {
    //send ihave packet back
    response(sock, from_packet, &from, fromlen, ids);
    int index = 0;
    int id = ids[index];

    while (index == 0 || id != 0)
    { 
      //receive GET packet
      spiffy_recvfrom(sock, from_packet, RECVBUFLEN, 0, (struct sockaddr *)&from, &fromlen);
      if (from_packet->header.magicnum == 15441 && from_packet->header.version == 1 
      && from_packet->header.packet_type == GET_PACK)
      { 
        char *masterData = malloc(BT_FILENAME_LEN);
        get_master_data(config.chunk_file,masterData);
        split_chunk_file(masterData, id);
        send_data_packet(sock, &from, sizeof(from));
        index++;
        id = ids[index];
        free(masterData);
      }
    }
  }
  free(from_packet);
}

void process_get(char *chunkfile, char *outputfile)
{
  printf("PROCESS GET SKELETON CODE CALLED.  Fill me in!  (%s, %s)\n",
         chunkfile, outputfile);
  header_t whohas_header;
  data_packet_t *whohas_packet = malloc(sizeof(data_packet_t));
  memset(whohas_packet, 0, RECVBUFLEN);
  FILE *chunkFile;

  int id;

  int start = 4, length = 20;
  int nOfChunks = 0;

  unsigned char hash[20];
  unsigned char buffer[41]; //20 bytes hash string(each hex number is treated as a char)
  unsigned char v;
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
  {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  whohas_header.magicnum = 15441;
  whohas_header.version = 1;
  whohas_header.packet_type = WHOHAS_PACK;
  whohas_header.header_len = 16;
  whohas_header.seq_num = 0;
  whohas_header.ack_num = 0;

  whohas_packet->header = whohas_header;
  for (int i = 1; i <= 3; i++)
  {
    whohas_packet->data[i] = 0;
  }

  chunkFile = fopen(chunkfile, "r");

  while (fscanf(chunkFile, "%d %s", &id, buffer) == 2)
  {
    for (int i = 0; i < 20; i++)
    {
      if (sscanf((char *)(buffer) + 2 * i, "%2hhx", &v) != 1)
        break;
      hash[i] = v;
    }
    nOfChunks++;
    length += 20;
    memcpy(whohas_packet->data + start, hash, CHUNKSIZE);
    start += 20;
  }
  whohas_packet->header.packet_len = length;
  whohas_packet->data[0] = nOfChunks;
  bt_peer_t *node = config.peers;
  while (node != NULL)
  {
    char str[14]; //length of internet address in sockaddr
    inet_ntop(AF_INET, &node->addr.sin_addr, str, 14);
    if (node->id != config.identity)
    {
      spiffy_sendto(sockfd, (char *)whohas_packet, sizeof(data_packet_t),
                    0, (struct sockaddr *)(&node->addr), sizeof(node->addr));
    }
    node = node->next;
  }

  struct sockaddr_in from;
  socklen_t fromlen = sizeof(fromlen); // fromlen must be initialized to the size of the from.
  //Its value might be changed by recefrom function

  data_packet_t *from_packet = malloc(sizeof(data_packet_t));
  memset(from_packet, 0, RECVBUFLEN);

  spiffy_recvfrom(sockfd, from_packet, RECVBUFLEN, 0, (struct sockaddr *)&from, &fromlen);

  start = 4;
  if (from_packet->header.magicnum == 15441 && from_packet->header.version == 1 && from_packet->header.packet_type == IHAVE_PACK)
  {
    printf("ihave packet received\n"
           "Incoming message from %s:%d\n\n\n",
           inet_ntoa(from.sin_addr),
           ntohs(from.sin_port));
    for (int i = 0; i < from_packet->data[0]; i++, start += CHUNKSIZE)
    {
      char hash[CHUNKSIZE];
      memcpy(hash, from_packet->data + start, CHUNKSIZE);
      send_get_packet(hash, sockfd, &from, fromlen);
      receive_data_packet(sockfd, &from, fromlen, config.output_file);
    }
  }
  free(whohas_packet);
  free(from_packet);
}

void handle_user_input(char *line, void *cbdata)
{
  char chunkf[128], outf[128];

  bzero(chunkf, sizeof(chunkf));
  bzero(outf, sizeof(outf));

  if (sscanf(line, "GET %120s %120s", chunkf, outf))
  {
    if (strlen(outf) > 0)
    {
      strcpy(config.output_file,outf);
      process_get(chunkf, outf);
      
    }
  }
}

void peer_run(bt_config_t *config)
{
  int sock;
  struct sockaddr_in myaddr;
  fd_set readfds;
  struct user_iobuf *userbuf;

  if ((userbuf = create_userbuf()) == NULL)
  {
    perror("peer_run could not allocate userbuf");
    exit(-1);
  }

  if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1)
  {
    perror("peer_run could not create socket");
    exit(-1);
  }

  bzero(&myaddr, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY); //server socket binds to all the local ip address
  myaddr.sin_port = htons(config->myport);

  if (bind(sock, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1)
  {
    perror("peer_run could not bind socket");
    exit(-1);
  }

  spiffy_init(config->identity, (struct sockaddr *)&myaddr, sizeof(myaddr));

  //(*config).sockfd = sock;
  while (1)
  {
    int nfds;
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sock, &readfds);

    nfds = select(sock + 1, &readfds, NULL, NULL, NULL);

    if (nfds > 0)
    {
      if (FD_ISSET(sock, &readfds))
      {
        process_inbound_udp(sock);
      }

      if (FD_ISSET(STDIN_FILENO, &readfds))
      {
        process_user_input(STDIN_FILENO, userbuf, handle_user_input,
                           "Currently unused");
      }
    }
  }
}
//no comment accepted in chunkfile
void get_master_data(char *chunkfile, char *master_data) {
  FILE *chunk_file = fopen(chunkfile,"r");
  
  if(fscanf(chunk_file,"File: %s",master_data) != 1) {
    fprintf(stderr, "error with getting the name of master data\n");
  }
  fclose(chunk_file);
  return;
}
