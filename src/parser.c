#include "ft_ping.h"

void print_usage(void)
{
    printf("Usage: ft_ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf(" Options valid for all request types:\n");
    printf("  -v, --verbose              verbose output\n");
    printf("  -?, --help                 give this help list\n");
    printf("  -s <size>                  set data size (bytes)\n");
    printf("  --ttl <ttl>                set IP time to live\n");
    printf("  -n                         no reverse DNS (already default)\n");
    printf("  -r                         bypass routing tables (SO_DONTROUTE)\n");
    printf("  -p <pattern>               fill payload with hex pattern\n\n");
    printf("Mandatory or optional arguments to long options are also mandatory or optional\n");
    printf("for any corresponding short options.\n\n");
    printf("Report bugs to: <ton_email@student.42.fr>\n");
    exit(EXIT_SUCCESS);
}

static int hex_char_to_int(char c) 
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int parse_hex_pattern(t_ping *ping, const char *hexstr)
{
    int len = strlen(hexstr);
    if (len % 2 != 0) 
    {
        fprintf(stderr, "ft_ping: pattern must have even number of hex digits\n");
        return -1;
    }
    int byte_len = len / 2;
    if (byte_len > 256) 
    {
        fprintf(stderr, "ft_ping: pattern too long (max 256 bytes)\n");
        return -1;
    }
    for (int i = 0; i < byte_len; i++) 
    {
        int high = hex_char_to_int(hexstr[2*i]);
        int low = hex_char_to_int(hexstr[2*i+1]);
        if (high < 0 || low < 0) {
            fprintf(stderr, "ft_ping: invalid hex digit in pattern\n");
            return -1;
        }
        ping->opt.pattern_bytes[i] = (uint8_t)((high << 4) | low);
    }
    ping->opt.pattern_len = byte_len;
    return 0;
}

void parse_args(t_ping *ping, int ac, char **av)
{
    ping->opt.size = 56;
    ping->opt.interval = 1.0;
    ping->opt.ttl = 64;
    ping->opt.no_dns = false;
    ping->opt.dontroute = false;
    ping->opt.pattern_len = 0;
    ping->hostname = NULL;

    if (ac < 2) 
    {
        fprintf(stderr, "ft_ping: missing host operand\n");
        fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < ac; i++) 
    {
        if (av[i][0] == '-' && av[i][1] != '\0' && av[i][1] != '-') 
        {
            for (int j = 1; av[i][j] != '\0'; j++) 
            {
                char c = av[i][j];
                if (c == 'v')
                    ping->opt.verbose = true;
                else if (c == '?')
                    print_usage();
                else if (c == 'n')
                    ping->opt.no_dns = true;
                else if (c == 'r')
                    ping->opt.dontroute = true;
                else if (c == 's') 
                {
                    if (av[i][j+1] != '\0') 
                    {
                        ping->opt.size = atoi(&av[i][j+1]);
                        break;
                    } 
                    else if 
                    (i+1 < ac) 
                    {
                        ping->opt.size = atoi(av[++i]);
                        break;
                    } 
                    else 
                    {
                        fprintf(stderr, "ft_ping: option requires an argument -- 's'\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else if (c == 'p') 
                {
                    if (av[i][j+1] != '\0') 
                    {
                        if (parse_hex_pattern(ping, &av[i][j+1]) != 0)
                            exit(EXIT_FAILURE);
                        break;
                    } 
                    else if (i+1 < ac) 
                    {
                        if (parse_hex_pattern(ping, av[++i]) != 0)
                            exit(EXIT_FAILURE);
                        break;
                    } 
                    else 
                    {
                        fprintf(stderr, "ft_ping: option requires an argument -- 'p'\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else 
                {
                    fprintf(stderr, "ft_ping: invalid option -- '%c'\n", c);
                    fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else if (strcmp(av[i], "--ttl") == 0) 
        {
            if (i+1 < ac) 
            {
                ping->opt.ttl = atoi(av[++i]);
            } 
            else 
            {
                fprintf(stderr, "ft_ping: option '--ttl' requires an argument\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(av[i], "--verbose") == 0) 
        {
            ping->opt.verbose = true;
        }
        else if (strcmp(av[i], "--help") == 0) 
        {
            print_usage();
        }
        else if (av[i][0] != '-') 
        {
            if (!ping->hostname)
                ping->hostname = av[i];
        }
        else 
        {
            fprintf(stderr, "ft_ping: invalid option '%s'\n", av[i]);
            exit(EXIT_FAILURE);
        }
    }

    if (!ping->hostname) 
    {
        fprintf(stderr, "ft_ping: missing host operand\n");
        fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
        exit(EXIT_FAILURE);
    }
    if (ping->opt.size < 0 || ping->opt.size > 65507) 
    {
        fprintf(stderr, "ft_ping: packet size %d is too large. Maximum is 65507\n", ping->opt.size);
        exit(EXIT_FAILURE);
    }
    if (ping->opt.ttl < 1 || ping->opt.ttl > 255) 
    {
        fprintf(stderr, "ft_ping: ttl %d out of range (1-255)\n", ping->opt.ttl);
        exit(EXIT_FAILURE);
    }
}