#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "my_signal.h"
#include "my_socket.h"
#include "logUtil.h"
#include "set_cpu.h"
#include "readn.h"

int debug = 0;
int enable_quick_ack = 0;
int set_so_sndbuf_size = 0;
volatile sig_atomic_t has_usr1 = 0;

int child_proc(int connfd, int run_cpu, int use_no_delay, int server_read_bufsize, int server_write_bufsize)
{
    int n;
    
    unsigned char *read_buf = malloc(server_read_bufsize);
    if (read_buf == NULL) {
        err(1, "malloc for read_buf");
    }
    unsigned char *write_buf = malloc(server_write_bufsize);
    if (write_buf == NULL) {
        err(1, "malloc for write_buf");
    }

    if (use_no_delay) {
        fprintfwt(stderr, "use_no_delay\n");
        set_so_nodelay(connfd);
    }

    pid_t pid = getpid();
    fprintfwt(stderr, "server: pid: %d\n", pid);
    int so_snd_buf = get_so_sndbuf(connfd);
    fprintfwt(stderr, "server: SO_SNDBUF: %d (init)\n", so_snd_buf);

    if (run_cpu != -1) {
        if (set_cpu(run_cpu) < 0) {
            errx(1, "set_cpu");
        }
    }

    for ( ; ; ) {
        if (enable_quick_ack) {
#ifdef __linux__
            int qack = 1;
            setsockopt(connfd, IPPROTO_TCP, TCP_QUICKACK, &qack, sizeof(qack));
#endif
        }
        fprintfwt(stderr, "server: read start\n");
        n = read(connfd, read_buf, server_read_bufsize);
        if (n < 0) {
            err(1, "readn");
        }
        if (n == 0) {
            fprintfwt(stderr, "server: EOF\n");
            exit(0);
        }
        fprintfwt(stderr, "server: read %d bytes done\n", n);

        n = write(connfd, write_buf, server_write_bufsize);
        if (n < 0) {
            err(1, "write");
        }
    }

    return 0;
}

void sig_chld(int signo)
{
    pid_t pid;
    int   stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        ;
    }
    return;
}

int usage(void)
{
    char *msg =
"Usage: server\n"
"-s sleep_usec: sleep useconds after write\n"
"-p port:       port number (1234)\n"
"-q:            enable quick ack\n"
"-c run_cpu:    specify server run cpu (in child proc)\n"
"-D:            use TCP_NODELAY socket option\n";

    fprintf(stderr, "%s", msg);

    return 0;
}

int main(int argc, char *argv[])
{
    int port = 1234;
    pid_t pid;
    struct sockaddr_in remote;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int listenfd;
    int c;
    int run_cpu = -1;
    int use_no_delay = 0;
    int server_read_bufsize  = 1;
    int server_write_bufsize = 1;

    while ( (c = getopt(argc, argv, "b:B:c:dDhn:p:q")) != -1) {
        switch (c) {
            case 'b':
                server_read_bufsize = strtol(optarg, NULL, 0);
                break;
            case 'B':
                server_write_bufsize = strtol(optarg, NULL, 0);
                break;
            case 'c':
                run_cpu = strtol(optarg, NULL, 0);
                break;
            case 'd':
                debug += 1;
                break;
            case 'D':
                use_no_delay = 1;
                break;
            case 'h':
                usage();
                exit(0);
            case 'p':
                port = strtol(optarg, NULL, 0);
                break;
            case 'q':
                enable_quick_ack = 1;
                break;
            default:
                break;
        }
    }
    argc -= optind;
    argv += optind;

    my_signal(SIGCHLD, sig_chld);
    my_signal(SIGPIPE, SIG_IGN);

    listenfd = tcp_listen(port);
    if (listenfd < 0) {
        errx(1, "tcp_listen");
    }

    for ( ; ; ) {
        int connfd = accept(listenfd, (struct sockaddr *)&remote, &addr_len);
        if (connfd < 0) {
            err(1, "accept");
        }
        
        pid = fork();
        if (pid == 0) { //child
            if (close(listenfd) < 0) {
                err(1, "close listenfd");
            }
            if (child_proc(connfd, run_cpu, use_no_delay, server_read_bufsize, server_write_bufsize) < 0) {
                errx(1, "child_proc");
            }
            exit(0);
        }
        else { // parent
            if (close(connfd) < 0) {
                err(1, "close for accept socket of parent");
            }
        }
    }
        
    return 0;
}
