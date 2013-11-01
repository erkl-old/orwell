#ifndef __ORWELL_UTIL_H
#define __ORWELL_UTIL_H

#include <stdio.h>

/*
 * Reads a line from file into buf. Returns a non-zero error code
 * if the operation failed.
 */
int ow__readln(FILE *file, char *buf, size_t len);

/*
 * A chain is a bounded sequence of NULL-terminated strings. It allows
 * us to store an array of strings inside a user-provided buffers.
 */
struct ow__chain {
    char *head;
    char *tail;
    char *boundary;
};

int ow__chain_add(struct ow__chain *chain, const char *s);
char *ow__chain_find(struct ow__chain *chain, const char *needle);

#endif
