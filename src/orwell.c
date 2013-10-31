#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/sysinfo.h>
#include <sys/vfs.h>

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
    unsigned long long unit;

    int r = sysinfo(&info);
    if (r != 0) {
        return errno;
    }

    unit = (unsigned long long) info.mem_unit;

    mem->ram_total  = unit * ((unsigned long long) info.totalram);
    mem->ram_free   = unit * ((unsigned long long) info.freeram);
    mem->ram_shared = unit * ((unsigned long long) info.sharedram);
    mem->ram_buffer = unit * ((unsigned long long) info.bufferram);
    mem->swap_total = unit * ((unsigned long long) info.totalswap);
    mem->swap_free  = unit * ((unsigned long long) info.freeswap);

    return 0;
}

/*
 * Updates an ow_fs struct with data from statfs(2).
 */
int ow_read_fsutil(struct ow_fs *fs) {
    struct statfs stat;
    unsigned long long bsize;

    int r = statfs(fs->root, &stat);
    if (r != 0) {
        return errno;
    }

    bsize = stat.f_bsize;

    fs->capacity   = bsize * ((unsigned long long) stat.f_blocks);
    fs->free       = bsize * ((unsigned long long) stat.f_bfree);
    fs->available  = bsize * ((unsigned long long) stat.f_bavail);

    return 0;
}

/*
 * Updates an ow_netif_list struct with data from /proc/net/dev.
 */
int ow_read_netifs(struct ow_netif_list *netifs, struct ow_buf *buf) {
    int ret, n = 0;
    struct ow_netif *iface;

    FILE *file = fopen("/proc/net/dev", "r");
    if (file == NULL) {
        return errno;
    }

    netifs->len = 0;

    while ((ret = read_line(buf, file)) == 0 && !feof(file)) {
        /* the first two lines contain no useful information */
        if (n++ < 2) {
            continue;
        }

        if (netifs->len >= netifs->cap) {
            ret = EOVERFLOW;
            break;
        }

        iface = &netifs->interfaces[netifs->len++];

        sscanf(buf->base, " %[^:]: %llu %llu %llu %llu %llu %llu %llu %llu"
                                 " %llu %llu %llu %llu %llu %llu %llu %llu",
              iface->name,

              &iface->recv_bytes, &iface->recv_packets, &iface->recv_errs,
              &iface->recv_drop, &iface->recv_fifo, &iface->recv_frame,
              &iface->recv_compressed, &iface->recv_multicast,

              &iface->trans_bytes, &iface->trans_packets, &iface->trans_errs,
              &iface->trans_drop, &iface->trans_fifo, &iface->trans_colls,
              &iface->trans_carrier, &iface->trans_compressed);
    }

    fclose(file);
    return ret;
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
