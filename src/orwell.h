#ifndef __ORWELL_H
#define __ORWELL_H

/*
 * Placeholder struct definitions.
 */
struct ow_core_list;
struct ow_core;
struct ow_memory;

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
    unsigned long long swap_total;
    unsigned long long swap_free;
};

#endif
