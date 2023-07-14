#include <sys/ioctl.h>
#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "my_socket.h"
#include "readn.h"
#include "logUtil.h"
#include "set_cpu.h"

int usage()
{
    char msg[] = "Usage: client [options] ip_address\n"
                 "Connect to server and read data.  Display through put before exit.\n"
                 "\n"
                 "options:\n"
                 "-c CPU_NUM      running cpu num (default: none)\n"
                 "-p PORT         port number (default: 1234)\n"
                 "-q VALUE        specify quickack value (default: -1: use default value)\n"
                 "-n n_data       number of data (each 20 bytes) from server.  (default: 2)\n"
                 "-h              display this help\n";

    fprintf(stderr, "%s", msg);
    return 0;
}

int unset_so_quickack(int sockfd)
{
    int off = 0;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK , &off, sizeof(off)) < 0) {
        warn("unset setsockopt quickack");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int c;
    int n;
    char *remote;
    int port = 1234;
    int sleep_usec = 1000000;
    int cpu_num = -1;
    int sockfd;
    int n_data = 2;
    int quick_ack_value = -1;
    int client_write_bufsize = 1;
    int client_read_bufsize  = 1;

    while ( (c = getopt(argc, argv, "b:B:c:hn:p:q:s:")) != -1) {
        switch (c) {
            case 'b':
                client_write_bufsize = strtol(optarg, NULL, 0);
                break;
            case 'B':
                client_read_bufsize = strtol(optarg, NULL, 0);
                break;
            case 'c':
                cpu_num = strtol(optarg, NULL, 0);
                break;
            case 'h':
                usage();
                exit(0);
                break;
            case 'n':
                n_data = strtol(optarg, NULL, 0);
                break;
            case 'p':
                port = strtol(optarg, NULL, 0);
                break;
            case 'q':
                quick_ack_value = strtol(optarg, NULL, 0);
                break;
            case 's':
                sleep_usec = strtol(optarg, NULL, 0);
                break;
            default:
                break;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1) {
        usage();
        exit(1);
    }

    remote  = argv[0];
    if (cpu_num != -1) {
        if (set_cpu(cpu_num) < 0) {
            errx(1, "set_cpu fail: cpu_num %d", cpu_num);
        }
    }

    sockfd = tcp_socket();

    int rcvbuf = get_so_rcvbuf(sockfd);
    fprintfwt(stderr, "client: SO_RCVBUF: %d (init)\n", rcvbuf);

    if (connect_tcp(sockfd, remote, port) < 0) {
        errx(1, "connect_tcp");
    }
    
    unsigned char *write_buf = malloc(client_write_bufsize);
    if (write_buf == NULL) {
        err(1, "malloc for write_buf");
    }
    unsigned char *read_buf  = malloc(client_read_bufsize);
    if (read_buf == NULL) {
        err(1, "malloc for read_buf");
    }

    if (quick_ack_value != -1) {
        if (set_so_quickack(sockfd, quick_ack_value) < 0) {
            errx(1, "unset_so_quickack 0");
        }
    }

    int quick_ack;
    quick_ack = get_so_quickack(sockfd);
    fprintfwt(stderr, "client: quickack: %d\n", quick_ack);

    for (int i = 0; i < n_data; ++i) {
        if (quick_ack_value != -1) {
            if (set_so_quickack(sockfd, quick_ack_value) < 0) {
                errx(1, "set_so_quickack 0");
            }
        }

        fprintfwt(stderr, "client: write start\n");
        n = write(sockfd, write_buf, client_write_bufsize);
        if (n < 0) {
            err(1, "client: write");
        }
        if (n == 0) {
            fprintfwt(stderr, "client: write return 0\n");
            exit(0);
        }
        fprintfwt(stderr, "client: write done: return: %d\n", n);
        quick_ack = get_so_quickack(sockfd);
        fprintfwt(stderr, "client: quickack: %d\n", quick_ack);
        
        n = read(sockfd, read_buf, client_read_bufsize);
        if (n < 0) {
            err(1, "client: read");
        }
        if (n == 0) {
            fprintfwt(stderr, "EOF\n");
            exit(0);
        }
        fprintfwt(stderr, "client: read done: return: %d\n", n);
        quick_ack = get_so_quickack(sockfd);
        fprintfwt(stderr, "client: quickack: %d\n", quick_ack);

        fprintfwt(stderr, "client: entering sleep\n");
        usleep(sleep_usec);
        fprintfwt(stderr, "client: sleep done\n");
    }

    if (close(sockfd) < 0) {
        err(1, "close");
    }

    return 0;
}
