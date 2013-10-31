#ifndef __ORWELL_H
#define __ORWELL_H

#include <net/if.h>

/*
 * Placeholder struct definitions.
 */
struct ow_core_list;
struct ow_core;
struct ow_memory;
struct ow_fs;
struct ow_netif_list;
struct ow_netif;

/*
 * Holds a chunk of scratch space. These buffers let us to delegate
 * memory management to the user.
 */
struct ow_buf {
    char *base;
    size_t len;
};

/*
 * Reads usage statistics for all cores (measured in jiffies) from /proc/stat.
 * The result is stored in a preallocated ow_core_list struct, the len property
 * of which will be updated to reflect the number of cores found.
 *
 * If there are statistics for more CPU cores than there are elements
 * in list->base (the length of which must be indicated by list->cap),
 * ow_read_cores updates as many elements of the list->base array as possible
 * before returning EOVERFLOW.
 *
 * Additionally, ow_read_cores will return EOVERFLOW if any line in /proc/stat
 * is too long to fit into buf.
 */
int ow_read_cores(struct ow_core_list *list, struct ow_buf *buf);

struct ow_core_list {
    struct ow_core *base;
    int len;
    int cap;
};

struct ow_core {
    unsigned long long total;
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long virt;
};

/*
 * Snapshots current memory usage statistics using the sysinfo(2)
 * system call.
 */
int ow_read_memory(struct ow_memory *mem);

struct ow_memory {
    unsigned long long ram_total;
    unsigned long long ram_free;
    unsigned long long ram_shared;
    unsigned long long ram_buffer;
    unsigned long long swap_total;
    unsigned long long swap_free;
};

/*
 * Measures utilization of a particular disk using the statfs(2) system
 * call, storing the result in an ow_diskutil struct. The path parameter
 * can be any file within the mounted file system.
 */
int ow_read_fsutil(struct ow_fs *fs);

struct ow_fs {
    const char *root;
    const char *device;
    const char *type;

    unsigned long long capacity;
    unsigned long long free;
    unsigned long long available;
};

/*
 * Copies usage stats for up to netifs->cap network interfaces into the
 * preallocated array at netifs->interfaces, using the data available in
 * /proc/net/dev. Also updates netifs->len to reflect how many elements
 * in netifs->interfaces were updated.
 *
 * Returns an appropriate error code on file error. May also return
 * EOVERFLOW if more than netifs->cap interfaces were found, or if buf
 * is too small to hold each line in /proc/net/dev.
 */
int ow_read_netifs(struct ow_netif_list *netifs, struct ow_buf *buf);

struct ow_netif_list {
    struct ow_netif *interfaces;
    int len;
    int cap;
};

struct ow_netif {
    char name[IF_NAMESIZE];

    unsigned long long recv_bytes;
    unsigned long long recv_packets;
    unsigned long long recv_errs;
    unsigned long long recv_drop;
    unsigned long long recv_fifo;
    unsigned long long recv_frame;
    unsigned long long recv_compressed;
    unsigned long long recv_multicast;

    unsigned long long trans_bytes;
    unsigned long long trans_packets;
    unsigned long long trans_errs;
    unsigned long long trans_drop;
    unsigned long long trans_fifo;
    unsigned long long trans_colls;
    unsigned long long trans_carrier;
    unsigned long long trans_compressed;
};

#endif
