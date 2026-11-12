#include "ft_ping.h"

volatile sig_atomic_t g_stop = 0;
volatile sig_atomic_t g_ping_ready = 0;

void handle_sigint(int sig) 
{
    (void)sig;
    g_stop = 1;
}

void handle_sigalrm(int sig) 
{
    (void)sig;
    g_ping_ready = 1;
}

void setup_signals(void) 
{
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_sigint;
    if (sigaction(SIGINT, &sa_int, NULL) < 0) 
    {
        perror("ft_ping: échec configuration SIGINT");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa_alrm;
    memset(&sa_alrm, 0, sizeof(sa_alrm));
    sa_alrm.sa_handler = handle_sigalrm;
    if (sigaction(SIGALRM, &sa_alrm, NULL) < 0) 
    {
        perror("ft_ping: échec configuration SIGALRM");
        exit(EXIT_FAILURE);
    }
}

void schedule_next_ping(double interval) 
{
    struct itimerval value;

    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;
    
    value.it_value.tv_sec = (long)interval;
    value.it_value.tv_usec = (long)((interval - value.it_value.tv_sec) * 1000000.0);

    setitimer(ITIMER_REAL, &value, NULL);
}

void print_stats(t_ping *ping) 
{
    printf("\n--- %s ping statistics ---\n", ping->hostname);
    long lost = ping->stats.transmitted - ping->stats.received;
    double loss_pct = ping->stats.transmitted > 0 ? ((double)lost / ping->stats.transmitted) * 100.0 : 0.0;
    printf("%ld packets transmitted, %ld packets received, %.0f%% packet loss\n",
           ping->stats.transmitted, ping->stats.received, loss_pct);
    
    if (ping->stats.received > 0) 
    {
        double avg = ping->stats.rtt_sum / ping->stats.received;
        double variance = (ping->stats.rtt_sum2 / ping->stats.received) - (avg * avg);
        if (variance < 0) 
        {
            variance = 0.0;
        }
        double mdev = sqrt(variance);
        printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
               ping->stats.rtt_min, avg, ping->stats.rtt_max, mdev);
    }
}