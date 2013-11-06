#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../src/orwell.h"

int main(void) {
    struct ow_core prev[8], curr[8];
    char *buf[4096];
    unsigned long long dt, di;
    int n, i;

    /* prime the `prev` array */
    n = ow_read_cores(prev, sizeof prev, (char *) buf, sizeof buf);
    if (n < 0) {
        fprintf(stderr, "ow_read_cores error: %s\n", strerror(-n));
        return 1;
    }

    /* print column headers */
    for (i = 0; i < n; i++) {
        printf("  cpu%1d", i);
    }
    printf("\n");

    /* every second, print per-core utilization levels for the
     * last second */
    while (1) {
        sleep(1);

        n = ow_read_cores(curr, sizeof curr, (char *) buf, sizeof buf);
        if (n < 0) {
            fprintf(stderr, "ow_read_cores error: %s\n", strerror(-n));
            return 1;
        }

        while (n--) {
            dt = curr[n].total - prev[n].total;
            di = curr[n].idle - prev[n].idle;
            prev[n] = curr[n];

            printf("  %3llu%%", (dt - di) * 100 / dt);
        }
        printf("\n");
    }

    return 0;
}
