#include "window.h"
#include "time.h"
#include "dplist.h"

dplist_t *packets;
sending_window_t *window;
void window_free();
int isWindowEmpty();
void *element_copy(void *src_element)
{
    data_packet_t *packet = malloc(sizeof(data_packet_t));
    *packet = *((data_packet_t *)src_element);
    return packet;
}
void element_free(void **element)
{
    free(*element);
    *element = NULL;
}
int element_compare(void *x, void *y)
{
    data_packet_t *packet1 = (data_packet_t *)x;
    data_packet_t *packet2 = (data_packet_t *)y;
    if (packet1->header.magicnum == packet2->header.magicnum && packet1->header.version == packet2->header.version && packet1->header.packet_type == packet2->header.packet_type && packet1->header.packet_len == packet2->header.packet_len)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void window_init()
{
    window = malloc(sizeof(sending_window_t));
    memset(window, 0, sizeof(sending_window_t));
    window->window_size = WINDOWSIZE;
}
void send_data_packet(int sockfd, struct sockaddr_in *to_addr, socklen_t tolen)
{
    //FILE *output = open(outputfile, "w");
    int nOfBytes;
    window_init();
    window->LastPacketAvailable = dpl_size(packets);
    struct sockaddr_in from;
    socklen_t from_len = sizeof(struct sockaddr_in);
    data_packet_t *from_packet = malloc(sizeof(data_packet_t));

    for (int i = 0; i < window->window_size; i++)
    {
        data_packet_t *data_packet = (data_packet_t *)dpl_get_element_at_index(packets, -1);
        if (data_packet != NULL)
        {
            window->packets[i] = *data_packet;
        }
        else
        {
            memset(&(window->packets[i]), 0, sizeof(data_packet_t));
        }

        spiffy_sendto(sockfd, (char *)&window->packets[i], sizeof(data_packet_t), 0, (struct sockaddr *)to_addr, tolen);
#ifdef DEBUG_PACKET
        printf("Send DATA packet of sequence number %u\n", window->packets[i].header.seq_num);
#endif
        dpl_remove_at_index(packets, -1, true);
        (window->infos[i]).lastSent = time(NULL);
        window->LastPacketSent++;
        //window->next_insert = (window->next_insert + 1) % WINDOWSIZE;
    }
    //During transfering data, if we receive WHOHAS packet, we just ignore it
    while (!isWindowEmpty())
    { //receive ACK packet
        nOfBytes = spiffy_recvfrom(sockfd, (char *)from_packet, sizeof(data_packet_t), MSG_DONTWAIT, (struct sockaddr *)&from, &from_len);
        //TODO ignore the other ACK from other peer, possibility is low
        if (nOfBytes != 0 && from_packet->header.magicnum == MAGIC_NUMBER && from_packet->header.version == VERSION && from_packet->header.packet_type == ACK_PACK)
        {
#ifdef DEBUG_PACKET
            printf("Receive ACK packet of acknowledge number %u\n", from_packet->header.ack_num);
#endif
            int index = (from_packet->header.ack_num - 1) % window->window_size;
            //if acknowledge number equals the sequence number of the packet in packets[index]
            if (window->packets[index].header.seq_num == from_packet->header.ack_num)
            {
                //add 1 to index will probably result in overflow
                window->infos[(index + 1) % window->window_size].dupl_count++; //deplicate count should be added to the next packet
                data_packet_t *data_packet = (data_packet_t *)dpl_get_element_at_index(packets, -1);
                if (data_packet != NULL)
                {
                    window->packets[index] = *data_packet;
                    dpl_remove_at_index(packets, -1, true);
                    spiffy_sendto(sockfd, (char *)&window->packets[index], sizeof(data_packet_t), 0, (struct sockaddr *)to_addr, tolen);
#ifdef DEBUG_PACKET
                    printf("Send DATA packet of sequence number %u\n", window->packets[index].header.seq_num);
#endif
                    window->LastPacketSent = window->packets[index].header.seq_num;
                }
                else
                {
                    memset(&(window->packets[index]), 0, sizeof(data_packet_t));
                }
                window->LastAckReceived = from_packet->header.ack_num;
            }
            else
            {
                int next_index = (index + 1) % window->window_size;
                window->infos[next_index].dupl_count++;
                //if there are three duplicate packtes
                if (window->infos[next_index].dupl_count == DUPLIMIT)
                {
                    spiffy_sendto(sockfd, (char *)&window->packets[next_index], sizeof(data_packet_t), 0, (struct sockaddr *)to_addr, tolen);
#ifdef DEBUG_PACKET
                    printf("Send DATA packet of sequence number %u\n", window->packets[next_index].header.seq_num);
#endif
                    window->infos[next_index].dupl_count = 0;
                    window->infos[next_index].lastSent = time(NULL);
                }
            }
        }
        for (int i = 0; i < window->window_size; i++)
        {
            if (time(NULL) - (window->infos[i]).lastSent >= TIMEOUT && window->packets[i].header.seq_num != 0)
            {
                spiffy_sendto(sockfd, (char *)&window->packets[i], sizeof(data_packet_t), 0, (struct sockaddr *)to_addr, tolen);
#ifdef DEBUG_PACKET
                printf("Send DATA packet of sequence number %u\n", window->packets[i].header.seq_num);
#endif
                window->infos[i].dupl_count = 0;
                window->infos[i].lastSent = time(NULL);
            }
        }
    }
    dpl_free(&packets, true);
    window_free();
}

void split_chunk_file(char *filepath, int offset)
{
    FILE *data = fopen(filepath, "r");
    int i = 1, nbytes;
    long cumu_bytes = 0;
    packets = dpl_create(element_copy, element_free, element_compare);
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
    while (DATACHUNKSIZE - cumu_bytes > SENDBUFLEN && (nbytes = fread(packet->data, 1, SENDBUFLEN, data)) != 0)
    {
        packet->header.packet_len = PACK_HEADER_LEN + nbytes;
        packet->header.seq_num = i;
        i++;
        cumu_bytes += nbytes;
        dpl_insert_at_index(packets, packet, dpl_size(packets), true); //put the first element at the head of the dplist
    }
    nbytes = fread(packet->data, 1, DATACHUNKSIZE - cumu_bytes, data);
    packet->header.packet_len = PACK_HEADER_LEN + nbytes;
    packet->header.seq_num = i;
    i++;
    cumu_bytes += nbytes;
    dpl_insert_at_index(packets, packet, dpl_size(packets), true);

    fclose(data);
    free(packet);
}

void window_free()
{
    free(window);
    window = NULL;
}
void send_get_packet(char *hash, int sockfd, struct sockaddr_in *to_addr, socklen_t tolen)
{
    data_packet_t *get_packet = malloc(sizeof(data_packet_t));
    get_packet->header.magicnum = 15441;
    get_packet->header.version = 1;
    get_packet->header.packet_type = GET_PACK;
    get_packet->header.header_len = PACK_HEADER_LEN;
    get_packet->header.ack_num = 0;
    get_packet->header.seq_num = 0;
    get_packet->header.packet_len = PACK_HEADER_LEN + CHUNKSIZE;
    memcpy(get_packet->data, hash, CHUNKSIZE);
    spiffy_sendto(sockfd, (char *)get_packet, sizeof(data_packet_t), 0, (struct sockaddr *)to_addr, tolen);
#ifdef DEBUG_PACKET
    printf("Send GET packet \n");
#endif
    free(get_packet);
}
//TODO compare packet address with the IP address we are listenning
void receive_data_packet(int sockfd, struct sockaddr_in *from, socklen_t fromlen, char *outputFile)
{
    receiver_window_t *receive_window = malloc(sizeof(receiver_window_t));
    int nOfBytes;
    memset(receive_window, 0, sizeof(receiver_window_t));
    data_packet_t *from_packet = malloc(sizeof(data_packet_t));
    memset(from_packet, 0, PACKETLEN);
    struct sockaddr_in from_addr;
    socklen_t fromlen2 = sizeof(struct sockaddr_in);
    FILE *output = fopen(outputFile, "a");

    data_packet_t *ack_packet = malloc(sizeof(data_packet_t));
    ack_packet->header.magicnum = 15441;
    ack_packet->header.version = 1;
    ack_packet->header.packet_type = ACK_PACK;
    ack_packet->header.header_len = PACK_HEADER_LEN;
    ack_packet->header.seq_num = 0;
    ack_packet->header.packet_len = PACK_HEADER_LEN;

    while (receive_window->bytesReceived != DATACHUNKSIZE)
    {
        nOfBytes = spiffy_recvfrom(sockfd, from_packet, PACKETLEN, 0, (struct sockaddr *)&from_addr, &fromlen2);
/*#ifdef DEBUG_PACKET
        if (from_packet->header.seq_num == 354)
        {
            printf("end of the transfer\n");
        }
#endif*/
        header_t header = from_packet->header;
        if (nOfBytes != 0 && from_packet->header.magicnum == MAGIC_NUMBER && from_packet->header.version == VERSION && from_packet->header.packet_type == DATA_PACK)
        {
#ifdef DEBUG_PACKET
            printf("Receive DATA packet of sequence number %u\n", header.seq_num);
#endif
            if (from_packet->header.seq_num == receive_window->LastPacketReceived + 1)
            {   
                int nbytes_write;
                if(receive_window->bytesReceived + nOfBytes - PACK_HEADER_LEN > DATACHUNKSIZE) {
                    nbytes_write = DATACHUNKSIZE - receive_window->bytesReceived;
                }
                else 
                    nbytes_write = nOfBytes - PACK_HEADER_LEN;

                receive_window->bytesReceived += nbytes_write;
                receive_window->LastPacketReceived++;
                fwrite((char *)from_packet->data, nbytes_write, 1, output);
                ack_packet->header.ack_num = from_packet->header.seq_num;
                spiffy_sendto(sockfd, (char *)ack_packet, sizeof(data_packet_t), 0, (struct sockaddr *)&from_addr, fromlen2);
#ifdef DEBUG_PACKET
                printf("Send ack packet of ack number %u\n", ack_packet->header.ack_num);
#endif
            }
        }
    }
    fclose(output);
    free(ack_packet);
    free(from_packet);
}

int isWindowEmpty()
{
    int isEmpty = 1;
    data_packet_t *empty_packet = malloc(sizeof(data_packet_t));
    memset(empty_packet, 0, sizeof(data_packet_t));
    for (int i = 0; i < window->window_size; i++)
    {
        if (memcmp(&(window->packets[i]), empty_packet, sizeof(data_packet_t)))
        {
            isEmpty = 0;
        }
    }
    free(empty_packet);
    return isEmpty;
}