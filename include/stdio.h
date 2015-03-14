#ifndef _UCC_STDIO_H
#define _UCC_STDIO_H

#include <stdarg.h>
#include <stddef.h>

#ifdef NULL
#undef NULL
#endif

#define NULL 0
#define EOF (-1)
#define BUFSIZ 1024
#define OPEN_MAX 10

typedef struct _iobuf {
  int  cnt;    /* characters left */
  char *ptr;   /* next character position */
  char *base;  /* location of the buffer */
  int  flag;   /* mode of the file access */
  int  fd;     /* file descriptor */
} FILE;

extern FILE _iob[OPEN_MAX];

#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])

enum _flags {
  _READ  = 01,
  _WRITE = 02,
  _UNBUF = 04,
  _EOF   = 010,
  _ERR   = 020
};

int _fillbuf(FILE *);
int _flushbuf(int, FILE *);
int fflush(FILE *);

#define feof(p)     (((p)->flag & _EOF) != 0)
#define ferror(p)   (((p)->flag & _ERR) != 0)
#define fileno(p)   ((p)->fd)

#define getc(p) (fflush(NULL),                                \
                 --(p)->cnt >= 0                              \
                 ? (unsigned char) *(p)->ptr++ : _fillbuf(p))

#define putc(x, p) (--(p)->cnt >= 0 \
                ? *(p)->ptr++ = (x) : _flushbuf((x), p))

#define getchar()   getc(stdin)
#define putchar(x)  putc((x), stdout)

/*
typedef struct {
  int (*read)(void);
  int (*write)(int);
} FILE;
*/

int fputc(int, FILE *);
int fgetc(FILE *);
int puts(char *);
int fputs(char *, FILE *);
char *fgets(char *, int, FILE *);
char *gets(char *);


int printf(const char *, ...);
int fprintf(FILE *, const char *, ...);
int vprintf(const char *, va_list);
int vfprintf(FILE *, const char *, va_list);

#endif  /* stdio.h */
