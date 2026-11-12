#include "ft_ping.h"

static void print_ip_header_dump(struct iphdr *ip, struct icmphdr *icmp) 
{
    printf("IP Hdr Dump:\n ");
    
    uint16_t *ptr = (uint16_t *)ip;
    int ip_hlen = ip->ihl * 4;
    for (int i = 0; i < ip_hlen / 2; i++) 
    {
        printf("%04x ", ntohs(ptr[i]));
    }
    printf("\n");
    
    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data\n");
    
    uint16_t frag_off = ntohs(ip->frag_off);
    uint8_t flags = frag_off >> 13;
    uint16_t offset = frag_off & 0x1FFF;
    
    char src_ip[INET_ADDRSTRLEN];
    char dst_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip->saddr, src_ip, sizeof(src_ip));
    inet_ntop(AF_INET, &ip->daddr, dst_ip, sizeof(dst_ip));
    
    printf(" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %s  %s \n",
           ip->version, ip->ihl, ip->tos, ntohs(ip->tot_len), ntohs(ip->id),
           flags, offset, ip->ttl, ip->protocol, ntohs(ip->check),
           src_ip, dst_ip);
           
    printf("ICMP: type %d, code %d, size %d, id 0x%04x, seq 0x%04x\n",
           icmp->type, icmp->code, ntohs(ip->tot_len) - ip_hlen,
           ntohs(icmp->un.echo.id), ntohs(icmp->un.echo.sequence));
}

void receive_ping(t_ping *ping) 
{
    int max_recv_size = 65536; 
    uint8_t buffer[65536];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);
    struct timeval tv_recv;

    while (!g_stop && !g_ping_ready)
    {
        ssize_t ret = recvfrom(ping->sockfd, buffer, max_recv_size, 0, 
                               (struct sockaddr *)&from, &from_len);                          
        if (ret < 0) 
            return; 
        
        gettimeofday(&tv_recv, NULL);

        if (ret < (ssize_t)(sizeof(struct iphdr) + sizeof(struct icmphdr))) 
            continue;

        struct iphdr *ip = (struct iphdr *)buffer;
        int ip_hlen = ip->ihl * 4;
        struct icmphdr *icmp = (struct icmphdr *)(buffer + ip_hlen);

        if (icmp->type == ICMP_ECHOREPLY) 
        {
            if (ntohs(icmp->un.echo.id) == ping->id) 
            {
                char src_ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ip->saddr, src_ip_str, sizeof(src_ip_str));

                double rtt = 0.0;
                uint8_t *payload = (uint8_t *)icmp + sizeof(struct icmphdr);

                if (ret >= (ssize_t)(ip_hlen + sizeof(struct icmphdr) + sizeof(struct timeval))) 
                {
                    struct timeval tv_send;
                    memcpy(&tv_send, payload, sizeof(struct timeval));
                    rtt = (double)(tv_recv.tv_sec - tv_send.tv_sec) * 1000.0 + 
                          (double)(tv_recv.tv_usec - tv_send.tv_usec) / 1000.0;
                }

                ping->stats.received++;
                if (ping->stats.received == 1) 
                {
                    ping->stats.rtt_min = rtt;
                    ping->stats.rtt_max = rtt;
                } 
                else 
                {
                    if (rtt < ping->stats.rtt_min) ping->stats.rtt_min = rtt;
                    if (rtt > ping->stats.rtt_max) ping->stats.rtt_max = rtt;
                }
                ping->stats.rtt_sum += rtt;
                ping->stats.rtt_sum2 += rtt * rtt;

                if (!ping->opt.quiet) 
                {
                    if (ping->opt.timestamp) 
                    {
                        printf("[%ld.%06ld] ", tv_recv.tv_sec, tv_recv.tv_usec);
                    }
                    printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
                           ret - ip_hlen, src_ip_str, ntohs(icmp->un.echo.sequence), ip->ttl, rtt);
                           
                    if (ping->opt.audible) 
                    {
                        printf("\a");
                        fflush(stdout);
                    }
                }
                break;
            }
        } 
        else if (icmp->type == ICMP_TIME_EXCEEDED) 
        {
            if (ret >= (ssize_t)(ip_hlen + sizeof(struct icmphdr) + sizeof(struct iphdr) + sizeof(struct icmphdr))) 
            {
                struct iphdr *inner_ip = (struct iphdr *)(buffer + ip_hlen + sizeof(struct icmphdr));
                int inner_ip_hlen = inner_ip->ihl * 4;
                struct icmphdr *inner_icmp = (struct icmphdr *)(buffer + ip_hlen + sizeof(struct icmphdr) + inner_ip_hlen);
                
                if (ntohs(inner_icmp->un.echo.id) == ping->id) 
                {
                    char pr_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &ip->saddr, pr_ip, sizeof(pr_ip));
                    
                    printf("%ld bytes from %s: Time to live exceeded\n", 
                           ret - ip_hlen, pr_ip);
                    
                    if (ping->opt.verbose) 
                    {
                        print_ip_header_dump(inner_ip, inner_icmp);
                    }
                    break;
                }
            }
        }
    }
}