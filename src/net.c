#include "ft_ping.h"

int resolve_host(t_ping *ping) 
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    int ret = getaddrinfo(ping->hostname, NULL, &hints, &ping->res);
    if (ret != 0) 
    {
        fprintf(stderr, "ft_ping: unknown host\n");
        return (-1);
    }
    struct sockaddr_in *addr = (struct sockaddr_in *)ping->res->ai_addr;
    inet_ntop(AF_INET, &addr->sin_addr, ping->ip_str, sizeof(ping->ip_str));
    return (0);
}

int open_socket(t_ping *ping) 
{
    ping->sockfd = socket(ping->res->ai_family, SOCK_RAW, IPPROTO_ICMP);
    if (ping->sockfd < 0) 
    {
        fprintf(stderr, "ft_ping: Lacking privilege for raw socket.\n");
        return (-1);
    }   
    if (setsockopt(ping->sockfd, IPPROTO_IP, IP_TTL, &ping->opt.ttl, sizeof(ping->opt.ttl)) < 0) 
    {
        perror("ft_ping: setsockopt(IP_TTL)");
        close(ping->sockfd);
        return (-1);
    }   
    if (ping->opt.dontroute) 
    {
        int on = 1;
        if (setsockopt(ping->sockfd, SOL_SOCKET, SO_DONTROUTE, &on, sizeof(on)) < 0) 
        {
            perror("ft_ping: setsockopt(SO_DONTROUTE)");
        }
    } 
    struct timeval tv_timeout;
    tv_timeout.tv_sec = (long)ping->opt.interval;
    tv_timeout.tv_usec = (long)((ping->opt.interval - tv_timeout.tv_sec) * 1000000.0);
    if (tv_timeout.tv_sec == 0 && tv_timeout.tv_usec == 0) 
    {
        tv_timeout.tv_sec = 1;
    }
    if (setsockopt(ping->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout, sizeof(tv_timeout)) < 0) 
    {
        perror("ft_ping: setsockopt(SO_RCVTIMEO)");
        close(ping->sockfd);
        return (-1);
    }
    return (0);
}