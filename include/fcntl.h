#ifndef _UCC_FCNTL_H
#define _UCC_FCNTL_H

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

#ifndef _XV6_USER_H
int open(const char *, int);
#endif

#endif
