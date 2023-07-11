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

#define WRITE_BUFSIZE 1
#define READ_BUFSIZE  1

int usage()
{
    char msg[] = "Usage: client [options] ip_address\n"
                 "Connect to server and read data.  Display through put before exit.\n"
                 "\n"
                 "options:\n"
                 "-c CPU_NUM      running cpu num (default: none)\n"
                 "-p PORT         port number (default: 1234)\n"
                 "-n n_data       number of data (each 20 bytes) from server.  (default: 2)\n"
                 "-h              display this help\n";

    fprintf(stderr, "%s", msg);
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

    while ( (c = getopt(argc, argv, "c:hn:p:s:")) != -1) {
        switch (c) {
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
    
    unsigned char write_buf[WRITE_BUFSIZE];
    unsigned char read_buf[READ_BUFSIZE];
    for (int i = 0; i < n_data; ++i) {

        fprintfwt(stderr, "client: write start\n");
        n = write(sockfd, write_buf, WRITE_BUFSIZE);
        if (n < 0) {
            err(1, "client: write");
        }
        if (n == 0) {
            fprintfwt(stderr, "client: write return 0\n");
            exit(0);
        }
        fprintfwt(stderr, "client: write done: return: %d\n", n);
        
        n = read(sockfd, read_buf, READ_BUFSIZE);
        if (n < 0) {
            err(1, "client: read");
        }
        if (n == 0) {
            fprintfwt(stderr, "EOF\n");
            exit(0);
        }
        fprintfwt(stderr, "client: read done: return: %d\n", n);

        fprintfwt(stderr, "client: entering sleep\n");
        usleep(sleep_usec);
        fprintfwt(stderr, "client: sleep done\n");
    }

    if (close(sockfd) < 0) {
        err(1, "close");
    }

    return 0;
}
