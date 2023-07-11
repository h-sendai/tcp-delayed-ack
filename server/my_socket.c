#include "my_socket.h"

int tcp_socket(void)
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        warn("socket(AF_INET, SOCK_STREAM, 0)");
        return -1;
    }

    return sockfd;
}

int udp_socket(void)
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        warn("socket(AF_INET, SOCK_DGRAM, 0)");
        return -1;
    }

    return sockfd;
}

int connect_tcp_timeout(int sockfd, char *host, int port, int timeout_sec)
{
    struct sockaddr_in servaddr;
    struct sockaddr_in *resaddr;
    struct addrinfo    hints;
    struct addrinfo    *res;
    int err;
    struct timeval tm_out, orig_tm_out;
    socklen_t len;

    len = sizeof(orig_tm_out);
    if (getsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &orig_tm_out, &len) < 0) {
        warn("getsockopt() for restore SO_SNDTIMEO");
        return -1;
    }

    res = 0;
    memset((char *)&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    if ( (err = getaddrinfo(host, 0, &hints, &res)) != 0) {
        return -1;
    }

    resaddr = (struct sockaddr_in *)res->ai_addr;
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(port);
    servaddr.sin_addr   = resaddr->sin_addr;
    freeaddrinfo(res);

    tm_out.tv_sec  = timeout_sec;
    tm_out.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tm_out, sizeof(tm_out)) < 0) {
        warnx("socket SO_SNDTIMEO timeout set fail");
        return -1;
    }
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        /* warn("connect"); */
        if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &orig_tm_out, sizeof(orig_tm_out)) < 0) {
            warn("setsockopt to restore SO_SNDTIMEO");
        }
        errno = ETIMEDOUT;
        return -1;
    }

    /* restore SO_SNDTIMEO */
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &orig_tm_out, sizeof(orig_tm_out)) < 0) {
        warn("setsockopt to restore SO_SNDTIMEO");
        return -1;
    }

    return 0;
}

/* from kolc */
int connect_tcp(int sockfd, char *host, int port)
{
    struct sockaddr_in servaddr;
    struct sockaddr_in *resaddr;
    struct addrinfo    hints;
    struct addrinfo    *res;
    int err;

    res = 0;
    memset((char *)&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    if ( (err = getaddrinfo(host, 0, &hints, &res)) != 0) {
        return -1;
    }

    resaddr = (struct sockaddr_in *)res->ai_addr;
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(port);
    servaddr.sin_addr   = resaddr->sin_addr;
    freeaddrinfo(res);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        return -1;
    }
    //return connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    return 0;
}

int connect_udp(int sockfd, char *host, int port)
{
    struct sockaddr_in servaddr;
    struct sockaddr_in *resaddr;
    struct addrinfo    hints;
    struct addrinfo    *res;
    int err;

    res = 0;
    memset((char *)&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    if ( (err = getaddrinfo(host, 0, &hints, &res)) != 0) {
        return -1;
    }

    resaddr = (struct sockaddr_in *)res->ai_addr;
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(port);
    servaddr.sin_addr   = resaddr->sin_addr;
    freeaddrinfo(res);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        return -1;
    }
    //return connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    return 0;
}

int my_bind(int sockfd, char *host, int port)
{
    /* specify host "0.0.0.0" to specify INADDR_ANY */

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, host, &addr.sin_addr.s_addr);
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        warn("bind");
        return -1;
    }

    return 0;
}

int get_so_rcvbuf(int sockfd)
{
    int ret_so_rcvbuf;
    socklen_t len;
    
    len = sizeof(ret_so_rcvbuf);

    if(getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &ret_so_rcvbuf, &len) < 0) {
        warn("cannot get SO_RCVBUF");
        return -1;
    }

    return ret_so_rcvbuf;
}

int set_so_rcvbuf(int sockfd, int so_rcvbuf)
{
    socklen_t len;
    len = sizeof(so_rcvbuf);
    int ret_so_rcvbuf = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf, len) < 0) {
        warn("cannot set SO_RCVBUF");
        return -1;
    }

    if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &ret_so_rcvbuf, &len) < 0) {
        warn("cannot get SO_RCVBUF");
        return -1;
    }

    ret_so_rcvbuf = get_so_rcvbuf(sockfd);
#ifdef __linux__
    if (ret_so_rcvbuf != (2*so_rcvbuf)) {
#else
    if (ret_so_rcvbuf != so_rcvbuf) {
#endif
        warnx("cannot set SO_RCVBUF to %d bytes, but set %d bytes", so_rcvbuf, ret_so_rcvbuf);
        return -1;
    }

    return ret_so_rcvbuf;
}

int get_so_sndbuf(int sockfd)
{
    int ret_so_sndbuf;
    socklen_t len;
    
    len = sizeof(ret_so_sndbuf);

    if(getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &ret_so_sndbuf, &len) < 0) {
        warn("cannot get SO_SNDBUF");
        return -1;
    }

    return ret_so_sndbuf;
}

int set_so_sndbuf(int sockfd, int so_sndbuf)
{
    socklen_t len;
    len = sizeof(so_sndbuf);
    int ret_so_sndbuf = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &so_sndbuf, len) < 0) {
        warn("cannot set SO_RCVBUF");
        return -1;
    }

    if (getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &ret_so_sndbuf, &len) < 0) {
        warn("cannot get SO_SNDBUF");
        return -1;
    }

    ret_so_sndbuf = get_so_sndbuf(sockfd);
#ifdef __linux__
    if (ret_so_sndbuf != (2*so_sndbuf)) {
#else
    if (ret_so_sndbuf != so_sndbuf) {
#endif
        warnx("cannot set SO_SNDBUF to %d bytes, but set %d bytes", so_sndbuf, ret_so_sndbuf);
        return -1;
    }

    return ret_so_sndbuf;
}

int get_bytes_in_rcvbuf(int sockfd)
{
    int nbytes;
    if (ioctl(sockfd, FIONREAD, &nbytes) < 0) {
        warn("ioctl(sockfd, FIONREAD, &nbytes)");
    }

    return nbytes;
}

int set_so_nodelay(int sockfd)
{
    int on = 1;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY , &on, sizeof(on)) < 0) {
        warn("setsockopt nodelay");
        return -1;
    }

    return 0;
}

#ifdef __linux__
int set_so_quickack(int sockfd)
{
    int on = 1;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK , &on, sizeof(on)) < 0) {
        warn("setsockopt quickack");
        return -1;
    }

    return 0;
}
#endif

int get_so_rcvlowat(int sockfd)
{
    int size;
    socklen_t len;

    if (getsockopt(sockfd, SOL_SOCKET, SO_RCVLOWAT , &size, &len) < 0) {
        warn("getsockopt so_rcvlowat");
        return -1;
    }

    return size;
}

int set_so_rcvlowat(int sockfd, int lowat)
{
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVLOWAT , &lowat, sizeof(lowat)) < 0) {
        warn("setsockopt so_rcvlowat");
        return -1;
    }

    return 0;
}

int tcp_listen(int port)
{
    int on = 1;
    int listenfd;
    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(port);

    listenfd = tcp_socket();

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        warn("setsockopt");
        return -1;
    }

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        warn("bind");
        return -1;
    }

    if (listen(listenfd, 10) < 0) {
        warn("listen");
        return -1;
    }
    
    return listenfd;
}

int accept_connection(int port)
{
    struct sockaddr_in remote_addr;
    socklen_t addr_len = sizeof(remote_addr);

    int sockfd, listenfd;

    listenfd = tcp_listen(port);
    if (listenfd < 0) {
        fprintf(stderr, "tcp_listen");
        return -1;
    }
    
    sockfd = accept(listenfd, (struct sockaddr *) &remote_addr, &addr_len);
    if (sockfd < 0) {
        warn("accept");
        return -1;
    }

    return sockfd;
}

int get_port_num(int sockfd)
{
    struct sockaddr_in myaddr;
    socklen_t len = sizeof(myaddr);
    if (getsockname(sockfd, (struct sockaddr *)&myaddr, &len) < 0) {
        warn("getsockname");
        return -1;
    }

    return ntohs(myaddr.sin_port);
}

double MiB2Gb(double x)
{
    return x*1024.0*1024.0*8/1000000000.0;
}

int set_so_rcvtimeout(int sockfd, long tv_sec, long tv_usec)
{
    struct timeval timeout = { tv_sec, tv_usec };

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        warn("setsockopt SO_RCVTIMEO");
        return -1;
    }

    return 0;
}

