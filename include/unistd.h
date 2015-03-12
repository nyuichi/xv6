#ifndef _XV6_UNISTD_H
#define _XV6_UNISTD_H

#include <stddef.h>

ssize_t write(int, const void *, size_t);
ssize_t read(int, void *, size_t);
int close(int);

#endif  /* unistd.h */
