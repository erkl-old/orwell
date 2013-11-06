#ifndef __ORWELL_H
#define __ORWELL_H

#include <inttypes.h>
#include <net/if.h>

/*
 * Struct prototypes.
 */
struct ow_memory;
struct ow_core;
struct ow_fs;
struct ow_netif;

/*
 * Copies current memory utilization statistics to the provided struct.
 * Returns a negated error code on error.
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
 * Updates up to `cap` entries in the `dst` array with current values of all
 * available jiffy counters per core. Returns the number of cores found, or
 * a negated error code.
 */
int ow_read_cores(struct ow_core *dst, int cap, char *buf, size_t len);

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
 * Finds up to `cap` mounted, physical filesystems, and writes their basic
 * information as well as current capacity and I/O metrics to entries in the
 * `dst` array. Returns the number of filesystems found, or a negated error
 * code.
 */
int ow_read_mounts(struct ow_fs *dst, int cap, char *buf, size_t len);

struct ow_fs {
    dev_t device;
    const char *root;
    const char *type;

    unsigned long long capacity;
    unsigned long long free;
    unsigned long long available;

    unsigned long long read;
    unsigned long long written;
};

/*
 * Fetches the name and usage metrics for up to `cap` network interfaces,
 * storing the data in the `dst` array. Returns the number of network
 * interfaces identified, or a negated error code.
 */
int ow_read_netifs(struct ow_netif *dst, int cap, char *buf, size_t len);

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
