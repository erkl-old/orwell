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
