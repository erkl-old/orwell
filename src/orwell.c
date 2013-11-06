#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <mntent.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/vfs.h>

#include "orwell.h"
#include "orwell-util.h"

/*
 * Updates `ow_core` entries in `list` with data from `/proc/stat`.
 */
int ow_read_cores(struct ow_list *list, char *buf, size_t len) {
    int ret = 0;

    /* always reset the list's length */
    list->len = 0;

    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL) {
        return errno;
    }

    while ((ret = ow__readln(file, buf, len)) == 0 && !feof(file)) {
        /* while `proc/stat` contains all kinds of stats, we're only interested
         * in lines beginning with "cpu" and a decimal digit */
        if (strlen(buf) < 4 || buf[0] != 'c' || buf[1] != 'p' ||
            buf[2] != 'u' || buf[3] < '0' || buf[3] > '9') {
            continue;
        }

        if (list->len >= list->cap) {
            ret = EOVERFLOW;
            break;
        }

        /* append the core to the list */
        struct ow_core *core = &((struct ow_core *) list->base)[list->len++];

        /* different kernel versions will include different numbers of fields,
         * but luckily `sscanf` will set all missing fields to 0 for us */
        sscanf(buf, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu",
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
 * Updates an `ow_memory` struct with data from `sysinfo(2)`.
 */
int ow_read_memory(struct ow_memory *mem) {
    struct sysinfo info;
    int r = sysinfo(&info);
    if (r != 0) {
        return errno;
    }

    /* convert each measurement to bytes */
    unsigned long long unit = (unsigned long long) info.mem_unit;

    mem->ram_total  = unit * ((unsigned long long) info.totalram);
    mem->ram_free   = unit * ((unsigned long long) info.freeram);
    mem->ram_shared = unit * ((unsigned long long) info.sharedram);
    mem->ram_buffer = unit * ((unsigned long long) info.bufferram);
    mem->swap_total = unit * ((unsigned long long) info.totalswap);
    mem->swap_free  = unit * ((unsigned long long) info.freeswap);

    return 0;
}

/*
 * Populates the `ow_fs` array at `list->base` with an entry for each mounted,
 * physical filesystem identified. Filesystems are listed as they appear in
 * `/proc/mounts`.
 */
int ow_read_filesystems(struct ow_list *list, char *buf, size_t len) {
    int ret = 0;

    /* update the list's length before doing anything else */
    list->len = 0;

    /* keep a pointer to the beginning of the buffer */
    char *head = buf;

    /* our first order of business is to compile a list of filesystem
     * types associated with a physical, mounted device */
    FILE *file = fopen("/proc/filesystems", "r");
    if (file == NULL) {
        return errno;
    }

    while ((ret = ow__readln(file, buf, len)) == 0 && !feof(file)) {
        if (buf[0] == '\t') {
            /* the lines that make it through look something like this:
             * "\text4\n\0", but we're not interested in the leading tab or
             * the trailing newline */
            size_t num = strlen(buf) - 1;
            buf[num] = '\0';

            /* add this filesystem type to array; abort if there's not
             * enough room */
            if ((ret = ow__strpush(&buf, &len, buf+1)) != 0) {
                break;
            }
        }
    }

    fclose(file);

    if (ret != 0) {
        return ret;
    }

    /* mark the end of our embedded string array */
    char *tail = buf;

    /* now we work our way through `/proc/mounts`, cross-referencing each
     * entry's filesystem type with the embedded array we just compiled */
    file = setmntent("/proc/mounts", "r");
    if (file == NULL) {
        return errno;
    }

    struct mntent mnt;

    while (getmntent_r(file, &mnt, buf, len) != NULL) {
        if (list->len >= list->cap) {
            ret = EOVERFLOW;
            break;
        }

        /* ignore entries for filesystem types which aren't associated
         * with any physical device */
        const char *type = ow__strfind(head, tail, mnt.mnt_type);
        if (type == NULL) {
            continue;
        }

        /* move `mnt.mnt_dir` to the start of the buffer */
        size_t n = strlen(mnt.mnt_dir) + 1;
        memcpy(buf, mnt.mnt_dir, n);

        /* use `stat(2)` to detect the device's major and minor numbers */
        struct stat st;
        if (stat(buf, &st) != 0) {
            ret = errno;
            break;
        }

        /* append this filesystem to the list */
        struct ow_fs *fs = &((struct ow_fs *) list->base)[list->len++];

        fs->device    = st.st_dev;
        fs->root      = buf;
        fs->type      = type;
        fs->capacity  = 0;
        fs->free      = 0;
        fs->available = 0;
        fs->read      = 0;
        fs->written   = 0;

        /* advance our buffer pointer so that subsequent calls to `getmntent_r`
         * won't overwrite the last entry's `root` property, which we keep in
         * the user-provided buffer */
        buf += n;
        len -= n;
    }

    if (ferror(file)) {
        return errno;
    }

    endmntent(file);
    return ret;
}

/*
 * Updates an `ow_fs` struct with space utilization stats, taken
 * from `statfs(2)`.
 */
int ow_read_fsutil(struct ow_fs *fs) {
    struct statfs stat;
    int r = statfs(fs->root, &stat);
    if (r != 0) {
        return errno;
    }

    /* convert the measurements from blocks to bytes */
    unsigned long long bsize = (unsigned long long) stat.f_bsize;

    fs->capacity  = bsize * ((unsigned long long) stat.f_blocks);
    fs->free      = bsize * ((unsigned long long) stat.f_bfree);
    fs->available = bsize * ((unsigned long long) stat.f_bavail);

    return 0;
}

/*
 * Updates the filesystem's `read` and `written` properties with figures
 * from `/proc/diskstats`.
 */
int ow_read_fsio(struct ow_fs *fs, char *buf, size_t len) {
    int ret = 0;

    FILE *file = fopen("/proc/diskstats", "r");
    if (file == NULL) {
        return errno;
    }

    while ((ret = ow__readln(file, buf, len)) == 0) {
        if (feof(file)) {
            ret = ENODATA;
            break;
        }

        /* each line includes the device's major and minor numbers -
         * use these to identify the filesystem we're looking for */
        unsigned int maj, min;
        unsigned long long rs, ws;

        sscanf(buf, " %u %u %*s %*u %*u %llu %*u %*u %*u %llu",
               &maj, &min, &rs, &ws);

        if (maj == major(fs->device) && min == minor(fs->device)) {
            /* read and write statistics are measured in number of
             * 512-byte sectors read */
            fs->read    = rs * 512;
            fs->written = ws * 512;
            break;
        }
    }

    fclose(file);
    return ret;
}

/*
 * Updates the preallocated `ow_netif` array stored in `list->base` with
 * data from `/proc/net/dev`.
 */
int ow_read_netifs(struct ow_list *list, char *buf, size_t len) {
    int ret = 0;

    /* the list's `len` property should always be zeroed */
    list->len = 0;

    FILE *file = fopen("/proc/net/dev", "r");
    if (file == NULL) {
        return errno;
    }

    int n = 0;

    while ((ret = ow__readln(file, buf, len)) == 0 && !feof(file)) {
        /* the first two rows contain no useful data; only
         * table headers */
        if (n++ < 2) {
            continue;
        }

        if (list->len >= list->cap) {
            ret = EOVERFLOW;
            break;
        }

        /* create a new network interface entry (luckily `sscanf` does all the
         * heavy lifting for us) */
        struct ow_netif *iface = &((struct ow_netif *) list->base)[list->len++];

        sscanf(buf, " %[^:]: %llu %llu %llu %llu %llu %llu %llu %llu"
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
