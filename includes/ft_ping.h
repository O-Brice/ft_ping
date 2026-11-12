#ifndef FT_PING_H
#define FT_PING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <float.h>
#include <math.h>

typedef struct s_options {
    bool verbose;
    bool quiet;
    bool audible;
    bool timestamp;
    long count;
    int ttl;
    double interval;
    int size;
    bool no_dns;
    bool dontroute;
    uint8_t pattern_bytes[256];
    int pattern_len;
} t_options;

typedef struct s_stats {
    long transmitted;
    long received;
    double rtt_min;
    double rtt_max;
    double rtt_sum;
    double rtt_sum2;
} t_stats;

typedef struct s_ping {
    int sockfd;
    char *hostname;
    char ip_str[INET_ADDRSTRLEN];
    struct addrinfo *res;
    uint16_t id;
    uint16_t seq;
    t_options opt;
    t_stats stats;
} t_ping;

extern volatile sig_atomic_t g_stop;
extern volatile sig_atomic_t g_ping_ready;

void            parse_args(t_ping *ping, int ac, char **av);

int             resolve_host(t_ping *ping);
int             open_socket(t_ping *ping);

unsigned short  calcul_checksum(unsigned short *addr, int count);
void            icmp_request(t_ping *ping, uint8_t *packet_buffer);
void            send_ping(t_ping *ping);

void            receive_ping(t_ping *ping);

void            setup_signals(void);
void            handle_sigint(int sig);
void            handle_sigalrm(int sig);
void            schedule_next_ping(double interval);
void            print_stats(t_ping *ping);

void            ping_loop(t_ping *ping);

#endif