#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "my_socket.h"

int main(int argc, char *argv[])
{
    int sockfd = tcp_socket();

    int quick_ack = get_so_quickack(sockfd);
    printf("quick_ack: %d\n", quick_ack);

    return 0;
}
