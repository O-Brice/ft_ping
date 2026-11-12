#include "ft_ping.h"

unsigned short calcul_checksum(unsigned short *addr, int count)
{
    unsigned long sum = 0;
    while (count > 1) 
    {
        sum += *addr++;
        count -= 2;
    }
    if (count > 0) 
    {
        unsigned short left_over = 0;
        *(unsigned char *)&left_over = *(unsigned char *)addr;
        sum += left_over;
    }
    while (sum >> 16) 
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (unsigned short)(~sum);
}

void icmp_request(t_ping *ping, uint8_t *packet_buffer)
{
    struct icmphdr *icmp = (struct icmphdr *)packet_buffer;
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(ping->id);
    icmp->un.echo.sequence = htons(ping->seq);
    icmp->checksum = 0;
    
    uint8_t *payload = packet_buffer + sizeof(struct icmphdr);
    
    if (ping->opt.pattern_len > 0) 
    {
        int pattern_len = ping->opt.pattern_len;
        for (int i = 0; i < ping->opt.size; i++) 
        {
            payload[i] = ping->opt.pattern_bytes[i % pattern_len];
        }
    } 
    else 
    {
        for (int i = 0; i < ping->opt.size; i++) 
        {
            payload[i] = i & 0xFF;
        }
    }
    if (ping->opt.size >= (int)sizeof(struct timeval)) 
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        memcpy(payload, &tv, sizeof(struct timeval));
    }

    int total_packet_size = sizeof(struct icmphdr) + ping->opt.size;
    icmp->checksum = calcul_checksum((unsigned short *)packet_buffer, total_packet_size);
}

void send_ping(t_ping *ping) 
{
    int packet_size = sizeof(struct icmphdr) + ping->opt.size;
    uint8_t buffer[65536];

    memset(buffer, 0, packet_size);
    icmp_request(ping, buffer);
    
    if (sendto(ping->sockfd, buffer, packet_size, 0, 
               ping->res->ai_addr, ping->res->ai_addrlen) < 0) 
    {
        perror("ft_ping: sending packet");
        return;
    }
    ping->stats.transmitted++;
    ping->seq++;
}