#include <window.h>

void *element_copy(void *src_element);
void element_free(void **element);
int element_compare(void *x, void *y);

void send_data_packet(int sockfd, dplist_t *packets)
{
    sending_window_t window;
}

void split_chunk_file(char *filepath, int offset, dplist_t *list)
{
    FILE *data = fopen(filepath, "r");
    int i = 1,nbytes;
    long cumu_bytes = 0;
    //dplist uses deep copy
    data_packet_t *packet = malloc(sizeof(data_packet_t));
    if (fseek(data, DATACHUNKSIZE * offset, SEEK_SET) != 0)
    {
        printf("error with fseek\n");
        exit(EXIT_FAILURE);
    }
    packet->header.magicnum = 15441;
    packet->header.version = 1;
    packet->header.packet_type = DATA_PACK;
    packet->header.header_len = PACK_HEADER_LEN;
    packet->header.ack_num = 0;
    //generate data packet
    while (DATACHUNKSIZE - cumu_bytes > SENDBUFLEN && (nbytes = fread(packet->data, SENDBUFLEN, 1, data)) != 0)
    {
        packet->header.packet_len = PACK_HEADER_LEN + nbytes;
        packet->header.seq_num = i;
        i++;
        cumu_bytes += nbytes;
        dpl_insert_at_index(list,packet,-1,true);
    }
    nbytes = fread(packet->data, DATACHUNKSIZE - cumu_bytes, 1, data);
    packet->header.packet_len = PACK_HEADER_LEN + nbytes;
    packet->header.seq_num = i;
    i++;
    cumu_bytes += nbytes;
    dpl_insert_at_index(list,packet,-1,true);
    
    fclose(data);
    free(packet);
}