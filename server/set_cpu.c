#define _GNU_SOURCE

#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>

int set_cpu(int cpu_num)
{
    cpu_set_t set;

    CPU_ZERO(&set);
    CPU_SET(cpu_num, &set);

    if (sched_setaffinity(0, sizeof(cpu_set_t), &set) < 0) {
        warn("sched_setaffinity");
        return -1;
    }

    return 0;
}
