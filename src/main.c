#include "ft_ping.h"

int main(int ac, char **av) 
{
    t_ping ping;
    
    memset(&ping, 0, sizeof(t_ping));
    ping.id = (uint16_t)(getpid() & 0xFFFF);
    
    parse_args(&ping, ac, av);
    
    if (resolve_host(&ping) < 0) 
    {
        return (EXIT_FAILURE);
    }
    
    if (open_socket(&ping) < 0) 
    {
        if (ping.res) 
        {
            freeaddrinfo(ping.res);
        }
        return (EXIT_FAILURE);
    }
    
    ping_loop(&ping);
    print_stats(&ping);
    
    if (ping.res) 
    {
        freeaddrinfo(ping.res);
    }
    if (ping.sockfd >= 0) 
    {
        close(ping.sockfd);
    }
    
    return (EXIT_SUCCESS);
}