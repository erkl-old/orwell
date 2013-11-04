#ifndef __ORWELL_UTIL_H
#define __ORWELL_UTIL_H

#include <stdio.h>

/*
 * Reads a line from `file` into `buf`. Returns IO-related errors, or
 * EOVERFLOW if the next line is more than `len` bytes long.
 */
int ow__readln(FILE *file, char *buf, size_t len);

/*
 * These functions manage lists of NULL-terminated strings, stored using a
 * limited amount of memory. They allow us to manage lists of strings inside
 * user-provided
 *
 * The `head` and `tail` pointers, and the `cap` value, relate to each other
 * the following way (illustrated here as a list containing the strings "foo"
 * and "bar"):
 *
 *     "foo\0bar\0<unused space>"
 *      ^         ^             ^
 *    head       tail      tail + cap
 */
int ow__strpush(char **tail, size_t *cap, const char *s);
const char *ow__strfind(const char *head, const char *tail, const char *needle);

#endif
