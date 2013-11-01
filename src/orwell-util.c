#include <errno.h>
#include <stdio.h>

#include "orwell-util.h"

/*
 * Reads a line from file into buf.
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
 * Appends a copy of s to the end of the chain.
 */
int ow__chain_add(struct ow__chain *ch, const char *s) {
    size_t index = 0;
    size_t limit = ch->boundary - ch->tail;
    unsigned char c;

    do {
        if (index >= limit) {
            return EOVERFLOW;
        }

        ch->tail[index] = (c = s[index]);
        index++;
    } while (c != '\0');

    ch->tail += index;
    return 0;
}

/*
 * Searches the chain for needle, returning the position of the
 * matching entry if found.
 */
char *ow__chain_find(struct ow__chain *ch, const char *needle) {
    char *curr = ch->head;
    size_t i;

    while (curr < ch->tail) {
        for (i = 0; ; i++) {
            if (curr[i] != needle[i]) {
                while (curr[i++] != '\0');
                break;
            }

            if (needle[i] == '\0') {
                return curr;
            }
        }

        curr += i;
    }

    return NULL;
}
