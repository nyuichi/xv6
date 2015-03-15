#ifndef _UCC_STRING_H
#define _UCC_STRING_H

#include <stddef.h>

size_t strlen(const char *);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);
char *strchr(const char *, int);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, size_t);
void *memset(void *, int, size_t);
void *memcpy(void *, const void *, size_t);
void *memmove(void*, const void*, size_t);

#endif  /* string.h */
