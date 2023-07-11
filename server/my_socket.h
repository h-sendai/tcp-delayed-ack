#ifndef _MY_SOCKET
#define _MY_SOCKET

#include <sys/wait.h>
#include <sys/stat.h>    /* for S_xxx file mode constants */
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>     /* for iovec{} and readv/writev */
#include <sys/ioctl.h>

#include <arpa/inet.h>   /* inet(3) functions */
#include <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include <netinet/tcp.h> /* for TCP_NODELAY */

#include <err.h>
#include <errno.h>
#include <fcntl.h>       /* for nonblocking */
#include <netdb.h>		 /* for getaddrinfo() */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

extern int tcp_socket(void);
extern int udp_socket(void);
extern int connect_tcp(int sockfd, char *host, int port);
extern int connect_tcp_timeout(int sockfd, char *host, int port, int timeout);
extern int connect_udp(int sockfd, char *host, int port);
extern int my_bind(int sockfd, char *host, int port);
extern int get_so_rcvbuf(int sockfd);
/* set_so_rcvbuf set SO_RCVBUF.  Re-examine the value and returns SO_RCVBUF value */
extern int set_so_rcvbuf(int sockfd, int so_rcvbuf);
extern int get_so_sndbuf(int sockfd);
extern int set_so_sndbuf(int sockfd, int so_sndbuf);
extern int get_bytes_in_rcvbuf(int sockfd);
extern int set_so_nodelay(int sockfd);
#ifdef __linux__
extern int set_so_quickack(int sockfd);
#endif
extern int get_so_rcvlowat(int sockfd);
extern int set_so_rcvlowat(int sockfd, int lowat);
extern int set_so_rcvtimeout(int sockfd, long tv_sec, long tv_usec);

extern int tcp_listen(int port);
extern int accept_connection(int port);

extern int get_port_num(int sockfd);
/* convert MiB (1024 base) to Giga (1000 base) bit */
extern double MiB2Gb(double x);
#endif
