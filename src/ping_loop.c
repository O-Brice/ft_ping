#include "ft_ping.h"

void ping_loop(t_ping *ping) 
{
    setup_signals();
    
    if (ping->opt.verbose) 
    {
        printf("PING %s (%s): %d data bytes, id 0x%04x = %d\n", 
               ping->hostname, 
               ping->ip_str, 
               ping->opt.size, 
               ping->id, 
               ping->id);
    } 
    else 
    {
        printf("PING %s (%s): %d data bytes\n", 
               ping->hostname, 
               ping->ip_str, 
               ping->opt.size);
    }

    g_ping_ready = 1; 

    while (!g_stop) 
    {
        if (g_ping_ready) 
        {
            send_ping(ping);
            g_ping_ready = 0;
            if (ping->opt.count > 0 && ping->stats.transmitted >= ping->opt.count) 
            {
                break;
            }
            schedule_next_ping(ping->opt.interval);
        }
        receive_ping(ping);
    }
}