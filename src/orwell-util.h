#ifndef __ORWELL_UTIL_H
#define __ORWELL_UTIL_H

#include <stdio.h>

/*
 * Reads a line from file into buf. Returns a non-zero error code
 * if the operation failed.
 */
int ow__readln(FILE *file, char *buf, size_t len);
