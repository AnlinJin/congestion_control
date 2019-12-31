/*
 * bt_parse.h
 *
 * Initial Author: Ed Bardsley <ebardsle+441@andrew.cmu.edu>
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2 command line and config file parsing
 * stubs.
 *
 */

#ifndef _BT_PARSE_H_
#define _BT_PARSE_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BT_FILENAME_LEN 255
#define BT_MAX_PEERS 1024

//peer node type, which consists of peer id, peer's socket (ip address + port)
typedef struct bt_peer_s {
  short  id;
  struct sockaddr_in addr;
  struct bt_peer_s *next;
} bt_peer_t;

struct bt_config_s {
  char  chunk_file[BT_FILENAME_LEN];
  char  has_chunk_file[BT_FILENAME_LEN];
  char  output_file[BT_FILENAME_LEN];
  char  peer_list_file[BT_FILENAME_LEN];
  int   max_conn;
  short identity;
  unsigned short myport;
  int sockfd;

  int argc; 
  char **argv;

  bt_peer_t *peers;
};
typedef struct bt_config_s bt_config_t;


void bt_init(bt_config_t *c, int argc, char **argv);
void bt_parse_command_line(bt_config_t *c);
//use peer_list file to generate a single linked list of peer node
void bt_parse_peer_list(bt_config_t *c);
void bt_dump_config(bt_config_t *c);
//get peer node based on its peer_id
bt_peer_t *bt_peer_info(const bt_config_t *c, int peer_id);

#endif /* _BT_PARSE_H_ */
