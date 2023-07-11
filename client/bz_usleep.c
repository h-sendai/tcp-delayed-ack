#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static long long gettime_ll(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) < 0) {
        warn("gettimeofday in gettime_ll");
        return -1;
    }
    
    return tv.tv_sec*1E6 + tv.tv_usec;
}

int bz_usleep(int usec)
{
    int loop = 0;
    int debug = 0;

    long long target = gettime_ll();
    target += usec;

    while (gettime_ll() < target) {
        if (debug) {
            loop ++;
        }
    }

    if (debug) {
        fprintf(stderr, "loop: %d\n", loop);
    }

    return 0;
}

#ifdef USE_MAIN
int main(int argc, char *argv[])
{
    struct timeval tv0, tv1, tvdiff;
    int usec;
    
    if (argc != 2) {
        fprintf(stderr, "Usage: bz_usleep usec\n");
        exit(1);
    }
    usec = strtol(argv[1], NULL, 0);

    gettimeofday(&tv0, NULL);
    bz_usleep(usec);
    gettimeofday(&tv1, NULL);
    timersub(&tv1, &tv0, &tvdiff);
    printf("%ld\n", tvdiff.tv_sec*1000000 + tvdiff.tv_usec);

    return 0;
}
#endif
