#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/sysinfo.h>

#include "orwell.h"

/*
 * Utility function prototypes.
 */
static int read_line(struct ow_buf *buf, FILE *f);

/*
 * Updates an ow_core_list struct with data from /proc/stat.
 */
int ow_read_cores(struct ow_core_list *list, struct ow_buf *buf) {
    int ret;
    struct ow_core *core;

    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL) {
        return errno;
    }

    list->len = 0;

    while ((ret = read_line(buf, file)) == 0 && !feof(file)) {
        /* we're only interested in lines beginning with "cpu[0-9]" */
        if (strlen(buf->base) < 4 || buf->base[0] != 'c' ||
            buf->base[1] != 'p' || buf->base[2] != 'u' ||
            buf->base[3] < '0' || buf->base[3] > '9') {
            continue;
        }

        if (list->len >= list->cap) {
            ret = EOVERFLOW;
            break;
        }

        core = &list->base[list->len++];

        sscanf(buf->base, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu",
            &core->user, &core->nice, &core->system,
            &core->idle, &core->iowait, &core->irq,
            &core->softirq, &core->steal, &core->virt);

        core->total =
            core->user + core->nice + core->system +
            core->idle + core->iowait + core->irq +
            core->softirq + core->steal + core->virt;
    }

    fclose(file);
    return ret;
}

/*
 * Updates an ow_memory struct with data from sysinfo(2).
 */
int ow_read_memory(struct ow_memory *mem) {
    struct sysinfo info;

    int r = sysinfo(&info);
    if (r != 0) {
        return r;
    }

    mem->ram_total  = (unsigned long long) info.totalram;
    mem->ram_free   = (unsigned long long) info.freeram;
    mem->swap_total = (unsigned long long) info.totalswap;
    mem->swap_free  = (unsigned long long) info.freeswap;

    return 0;
}

/*
 * Reads a line from file into buf. Returns an appropriate error code
 * if the operation failed.
 */
static int read_line(struct ow_buf *buf, FILE *file) {
    int i;

    if (fgets(buf->base, buf->len, file) == NULL) {
        return errno;
    }

    for (i = 0; ; i++) {
        if (i >= buf->len - 1) {
            return EOVERFLOW;
        }
        if (buf->base[i] == '\n') {
            break;
        }
    }

    return 0;
}
