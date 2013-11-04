#include <errno.h>
#include <stdio.h>

#include "orwell-util.h"

/*
 * Reads a line from `file` into `buf`. Returns IO-related errors, or
 * EOVERFLOW if the next line is more than `len` bytes long.
 */
int ow__readln(FILE *file, char *buf, size_t len) {
    int i;

    if (fgets(buf, len, file) == NULL) {
        return errno;
    }

    for (i = 0; ; i++) {
        if (i >= len - 1) {
            return EOVERFLOW;
        }
        if (buf[i] == '\n') {
            break;
        }
    }

    return 0;
}

/*
 * Appends a copy of s to the end of the string list, provided
 * `strlen(src) + 1 < cap` is true. The `tail` and `cap` pointers
 * will be updated accordingly if the operation succeeds.
 */
int ow__strpush(char **tail, size_t *cap, const char *src) {
    size_t i = 0;
    unsigned char c;

    do {
        if (i >= *cap) {
            return EOVERFLOW;
        }

        c = src[i];
        (*tail)[i++] = c;
    } while (c != '\0');

    *tail += i;
    *cap -= i;

    return 0;
}

/*
 * Searches the list between `head` and `tail` for `needle`, returning the
 * position of the matching entry if found.
 */
const char *ow__strfind(const char *head, const char *tail, const char *needle) {
    size_t i;

    for (; head < tail; head += i) {
        for (i = 0; ; i++) {
            if (head[i] != needle[i]) {
                while (head[i++] != '\0');
                break;
            }

            if (needle[i] == '\0') {
                return head;
            }
        }
    }

    return NULL;
}
