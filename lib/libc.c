#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#define max(m, n)   ((m) > (n) ? (m) : (n))
#define min(m, n)   ((m) < (n) ? (m) : (n))



/* assert.h */

void __assert_fail(const char *expr, const char *file, int line)
{
  printf("%s:%d: Assertion `%s' failed.\n", file, line, expr);
  abort();
}



/* ctype.h */

int isalnum(int c) { return isalpha(c) || isdigit(c); }
int isalpha(int c) { return islower(c) || isupper(c); }
int isdigit(int c) { return '0' <= c && c <= '9'; }
int islower(int c) { return 'a' <= c && c <= 'z'; }
int isspace(int c) { return c == ' ' || ('\t' <= c && c <= '\r'); }
int isupper(int c) { return 'A' <= c && c <= 'Z'; }
int tolower(int c) { return isupper(c) ? c - 'A' + 'a' : c; }
int toupper(int c) { return islower(c) ? c - 'a' + 'A' : c; }



/* setjmp.h */

int setjmp(jmp_buf buf)
{
  __asm("\
  mov r1, [rbp + 4]     # r1 <- buf                    \n\
  mov r2, [rbp]         # r2 <- caller rbp             \n\
  mov [r1], r2          # save caller rbp              \n\
  mov [r1 + 4], r28     # save r28 (return address)    \n\
  mov [r1 + 8], rbp     # save current rbp             \n\
  jl  r2, 0             # get PC                       \n\
  add r2, r2, 12        # r2 <- address of 'ret'       \n\
  mov [r1 + 12], r2     # save r2                      \n\
  mov r1, 0             # return 0                     \n\
  ret                                                  \n\
");
}

void longjmp(jmp_buf buf, int val)
{
  __asm("\
  mov r1, [rbp + 8]     # r1 <- val                   \n\
  cmpeq r2, r1, 0       # if (val == 0)               \n\
  add r1, r1, r2        #   r1 <- 1                   \n\
  mov r2, [rbp + 4]     # r2 <- buf                   \n\
  mov r3, [r2]          # r3 <- caller rbp            \n\
  mov r28, [r2 + 4]     # restore r28                 \n\
  mov rbp, [r2 + 8]     # restore setjmp rbp          \n\
  mov [rbp], r3         # restore setjmp's caller rbp \n\
  mov r2, [r2 + 12]     # r2 <- address of 'ret'      \n\
  jr  r2                # jump                        \n\
");
}



/* stdio.h */


FILE _iob[OPEN_MAX] = {     /* stdin, stdout, stderr */
  { 0, (char *) 0, (char *) 0, _READ, 0 },
  { 0, (char *) 0, (char *) 0, _WRITE | _LNBUF, 1 },
  { 0, (char *) 0, (char *) 0, _WRITE | _UNBUF, 2 }
};


FILE *fopen(const char *name, const char *mode)
{
  int fd;
  FILE *fp;

  if (*mode != 'r' && *mode != 'w' && *mode != 'a')
    return NULL;


  for (fp = _iob; fp < _iob + OPEN_MAX; fp++)
    if ( (fp->flag & (_READ | _WRITE)) == 0)
      break;  /* found free slot */

  if (fp >= _iob + OPEN_MAX)  /* no free slots */
    return NULL;

  if (*mode == 'w')
    fd = open(name, O_CREATE | O_WRONLY);
  else if (*mode == 'a') {
    if ((fd = open(name, O_WRONLY)) == -1)
      fd = open(name, O_WRONLY);
  } else
    fd = open(name, O_RDONLY);

  if (fd == -1)   /* couldn't access name */
    return NULL;

  fp->fd = fd;
  fp->cnt = 0;
  fp->base = NULL;
  fp->flag = (*mode == 'r')? _READ : _WRITE;
  return fp;

}

int fclose(FILE *fp) {

  fflush(fp);
  fp->flag = 0;
  free(fp->base);
  return close(fp->fd);
}


int _fillbuf(FILE *fp) {
  int bufsize;

  if ((fp->flag & (_READ | _EOF | _ERR)) != _READ)
    return EOF;

  bufsize = (fp->flag & _UNBUF) ? 1: BUFSIZ;

  if  (fp->base == NULL)  /* no buffer yet */
    if ((fp->base = (char *) malloc(bufsize)) == NULL)
      return EOF;         /* can't get buffer */

  fp->ptr = fp->base;
  fp->cnt = read(fp->fd, fp->ptr, bufsize);

  if (--fp->cnt < 0) {
    if (fp->cnt == -1)
      fp->flag |= _EOF;
    else
      fp->flag |= _ERR;
    fp->cnt = 0;
    return EOF;
  }

  return (unsigned char) *fp->ptr++;
}

int _flushbuf(int x, FILE *fp) {

  int num_written=0, bufsize=0;
  unsigned char uc = x;

  if ((fp->flag & (_WRITE|_EOF|_ERR)) != _WRITE)
    return EOF;
  if (fp->base == NULL && ((fp->flag & _UNBUF) == 0)) {
    /* no buffer yet */
    if ((fp->base = malloc(BUFSIZ)) == NULL) {
      /* couldn't allocate a buffer, so try unbuffered */
      fp->flag |= _UNBUF;
    } else {
      fp->ptr = fp->base;
      fp->cnt = BUFSIZ - 1;
    }
  }
  if (fp->flag & _UNBUF) {
    /* unbuffered write */
    fp->ptr = fp->base = NULL;
    fp->cnt = 0;
    if (x == EOF)
      return EOF;
    num_written = write(fp->fd, &uc, 1);
    bufsize = 1;
  } else {
    /* buffered write */
    assert(fp->ptr);
    if (x != EOF) {
      *fp->ptr++ = uc;
    }
    bufsize = (int)(fp->ptr - fp->base);
    while(bufsize - num_written > 0) {
      int t;
      t = write(fp->fd, fp->base + num_written, bufsize - num_written);
      num_written += t;
    }

    fp->ptr = fp->base;
    fp->cnt = BUFSIZ - 1;
  }

  if (num_written == bufsize) {
    return x;
  } else {
    fp->flag |= _ERR;
    return EOF;
  }
}

/* fflush */
int fflush(FILE *f)
{
  int retval;
  int i;

  retval = 0;
  if (f == NULL) {
    /* flush all output streams */
    for (i = 0; i < OPEN_MAX; i++) {
      if ((_iob[i].flag & _WRITE) && (fflush(&_iob[i]) == -1))
        retval = -1;
    }
  } else {
    if ((f->flag & _WRITE) == 0)
      return -1;
    _flushbuf(EOF, f);
    if (f->flag & _ERR)
      retval = -1;
  }
  return retval;
}


int fputc(int x, FILE *fp)
{
  int flush = (fp->flag & _LNBUF) && x == '\n';

  if (--fp->cnt >= 0 && !flush) {
    return *(fp)->ptr++ = x;
  } else {
    return _flushbuf(x, fp);
  }
}


int fgetc(FILE *fp)
{
  if (--fp->cnt >= 0) {
    return (unsigned char) *(fp)->ptr++;
  } else {
    return _fillbuf(fp);
  }

}

int fputs(char *s, FILE *stream)
{

  while(*s != '\0') {

    if (fputc(*s, stream) == EOF)
      return EOF;

    ++s;

  }

  return 1;

}


int puts(char *s)
{
  int i = 0;

  while(*s != '\0') {

    if (putchar(*s) == EOF)
      return EOF;

    ++i; ++s;

  }

  putchar('\n');

  return i + 1;
}



char *fgets(char *s, int size, FILE *stream)
{
  int c;
  char *buf;
  fflush(NULL);
  buf = s;

  if (size == 0)
    return NULL;

  while (--size > 0 && (c = fgetc(stream)) != EOF) {

    if ((*buf++ = c) == '\n')
      break;

  }

  *buf = '\0';

  return (c == EOF && buf == s) ? NULL : s;

}

char *gets(char *s)
{
  int c;
  char *buf;
  fflush(NULL);
  buf = s;

  while ((c = getc(stdin)) != EOF && c != '\n') {

    *buf++ = c;

  }

  *buf = '\0';

  return (c == EOF && buf == s) ? NULL : s;

}





#define LEFT      (1 << 0)
#define ZEROPAD   (1 << 1)
#define SIGN      (1 << 2)
#define PLUS      (1 << 3)
#define SPACE     (1 << 4)
#define OCTPRE    (1 << 5)
#define HEXPRE    (1 << 6)
#define CAP       (1 << 7)
#define DOT       (1 << 8)
#define TRUNC     (1 << 9)

static int prints(FILE *fp, const char *str, int w, int prec, int flag)
{
  int i, len;

  if (! str)
    str = "(null)";

  len = strlen(str);;
  if (prec >= 0 && len > prec)
    len = prec;

  if (! (flag & LEFT))
    for (i = 0; i < w - len; ++i)
      putc(' ', fp);

  for (i = 0; i < len; ++i)
    putc(str[i], fp);

  if (flag & LEFT)
    for (i = 0; i < w - len; ++i)
      putc(' ', fp);

  return max(len, w);
}

static int printi(FILE *fp, int x, int base, int w, int prec, int flag)
{
  char buf[12], pbuf[2];
  int i, n, len = 0, plen = 0;
  unsigned u = x;

  if (prec >= 0)
    flag &= ~ZEROPAD;

  if (flag & SIGN) {
    if (x < 0) {
      u = -x;
      pbuf[plen++] = '-';
    } else if (flag & (PLUS | SPACE)) {
      pbuf[plen++] = flag & PLUS ? '+' : ' ';
    }
  }
  if (x && flag & HEXPRE) {
    pbuf[plen++] = '0';
    pbuf[plen++] = flag & CAP ? 'X' : 'x';
  }

  while (u > 0) {
    int t = u % base;
    buf[len++] = (t < 10) ? (t + '0') : (t - 10 + (flag & CAP ? 'A' : 'a'));
    u /= base;
  }
  if (x == 0 || flag & OCTPRE)
    buf[len++] = '0';

  n = max(len, prec) + plen;

  if (! (flag & (LEFT | ZEROPAD)))
    for (i = 0; i < w - n; ++i)
      putc(' ', fp);

  for (i = 0; i < plen; ++i)
    putc(pbuf[i], fp);

  if (flag & ZEROPAD)
    for (i = 0; i < w - n; ++i)
      putc('0', fp);

  for (i = 0; i < prec - len; ++i)
    putc('0', fp);

  for (i = len - 1; i >= 0; --i)
    putc(buf[i], fp);

  if (flag & LEFT)
    for (i = 0; i < w - n; ++i)
      putc(' ', fp);

  return max(w, n);
}

/*
static float normalize(float f, int *e)
{
  static float table_p[] = { 1e1, 1e2, 1e4, 1e8, 1e16, 1e32 };
  static float table_m[] = { 1e-1, 1e-2, 1e-4, 1e-8, 1e-16, 1e-32 };
  static float table_m1[] = { 1e0, 1e-1, 1e-3, 1e-7, 1e-15, 1e-31 };

  int i;

  *e = 0;
  for (i = 5; i >= 0; --i) {
    if (f >= table_p[i]) {
      *e += 1 << i;
      f *= table_m[i];
    }
    if (f < table_m1[i]) {
      *e -= 1 << i;
      f *= table_p[i];
    }
  }

  return f;
}

static int printef(FILE *fp, float f, int w, int prec, int flag)
{
  char buf[10], pbuf[1];
  int i, n, expo = 0, len = 0, plen = 0, sp = 0;
  union { float f; unsigned i; } u;

  u.f = f;

  if (prec < 0)
    prec = 6;

  if (u.i >> 31) {
    f = -f;
    pbuf[plen++] = '-';
  } else if (flag & (PLUS | SPACE)) {
    pbuf[plen++] = flag & PLUS ? '+' : ' ';
  }

  if ((u.i >> 23 & 255) == 255) {
    int ofs = flag & CAP ? 'A' - 'a' : 0;
    sp = 1;
    flag &= ~ZEROPAD;
    if ((u.i & 0x7fffff) == 0) {
      buf[len++] = 'i' + ofs;
      buf[len++] = 'n' + ofs;
      buf[len++] = 'f' + ofs;
    } else {
      buf[len++] = 'n' + ofs;
      buf[len++] = 'a' + ofs;
      buf[len++] = 'n' + ofs;
    }
  } else if (f == 0.0) {
    buf[len++] = '0';
    buf[len++] = '.';
    // prec >= 9 is treated as prec = 8
    for (i = 0; i < min(prec, 8); ++i)
      buf[len++] = '0';
  } else {
    int x = 0;
    f = normalize(f, &expo);

    len = min(prec, 8) + 2;
    for (i = 0; i < len - 1; ++i) {
      int d = f;
      x = x * 10 + d;
      f = (f - d) * 10.0;
    }
    if (f >= 5.0) ++x;

    buf[1] = '.';
    for (i = len - 1; i >= 2; --i) {
      buf[i] = x % 10 + '0';
      x /= 10;
    }
    if (x == 10) {
      ++expo;
      buf[0] = '1';
    } else {
      buf[0] = x + '0';
    }
  }

  if (flag & TRUNC)
    while (buf[len - 1] == '0')
      --len;
  if (buf[len - 1] == '.' && ~flag & DOT)
    --len;

  n = len + plen + (sp || flag & TRUNC ? 0 : max(prec - 8, 0)) + (sp ? 0 : 4);

  if (! (flag & (LEFT | ZEROPAD)))
    for (i = 0; i < w - n; ++i)
      putc(' ', fp);

  for (i = 0; i < plen; ++i)
    putc(pbuf[i], fp);

  if (flag & ZEROPAD)
    for (i = 0; i < w - n; ++i)
      putc('0', fp);

  for (i = 0; i < len; ++i)
    putc(buf[i], fp);

  if (! (sp || flag & TRUNC))
    for (i = 0; i < prec - 8; ++i)
      putc('0', fp);

  if (! sp) {
    putc(flag & CAP ? 'E' : 'e', fp);
    putc(expo < 0 ? '-' : '+', fp);
    if (expo < 0) expo = -expo;
    putc(expo / 10 + '0', fp);
    putc(expo % 10 + '0', fp);
  }

  if (flag & LEFT)
    for (i = 0; i < w - n; ++i)
      putc(' ', fp);

  return max(w, n);
}

static int printff(FILE *fp, float f, int w, int prec, int flag)
{
  static float round_table[] = {
    5e-1, 5e-2, 5e-3, 5e-4, 5e-5, 5e-6, 5e-7, 5e-8, 5e-9, 5e-10
  };

  char buf[49], pbuf[1];
  int i, n, len = 0, plen = 0, sp = 0;
  union { float f; unsigned i; } u;
  __asm("break 2\n");

  u.f = f;

  if (prec < 0)
    prec = 6;

  if (u.i >> 31) {
    f = -f;
    pbuf[plen++] = '-';
  } else if (flag & (PLUS | SPACE)) {
    pbuf[plen++] = flag & PLUS ? '+' : ' ';
  }
  __asm("break 3\n");

  if ((u.i >> 23 & 255) == 255) {
    int ofs = flag & CAP ? 'A' - 'a' : 0;
    sp = 1;
    flag &= ~ZEROPAD;
    if ((u.i & 0x7fffff) == 0) {
      buf[len++] = 'i' + ofs;
      buf[len++] = 'n' + ofs;
      buf[len++] = 'f' + ofs;
    } else {
      buf[len++] = 'n' + ofs;
      buf[len++] = 'a' + ofs;
      buf[len++] = 'n' + ofs;
    }
  } else {
    int expo;
    float g;

    // prec >= 10 is treated as prec = 9
    f += round_table[min(prec, 9)];

    g = normalize(f, &expo);
    if (expo < 0)
      expo = 0;
    else
      f = g;

    for (; expo >= 0; --expo) {
      int d = f;
      buf[len++] = d + '0';
      f = (f - d) * 10.0;
    }

    buf[len++] = '.';
    for (i = 0; i < min(prec, 9); ++i) {
      int d = f;
      buf[len++] = d + '0';
      f = (f - d) * 10.0;
    }
  }

  if (flag & TRUNC)
    while (buf[len - 1] == '0')
      --len;
  if (buf[len - 1] == '.' && ~flag & DOT)
    --len;

  n = len + plen + (sp || flag & TRUNC ? 0 : max(prec - 9, 0));

  if (! (flag & (LEFT | ZEROPAD)))
    for (i = 0; i < w - n; ++i)
      putc(' ', fp);

  for (i = 0; i < plen; ++i)
    putc(pbuf[i], fp);

  if (flag & ZEROPAD)
    for (i = 0; i < w - n; ++i)
      putc('0', fp);

  for (i = 0; i < len; ++i)
    putc(buf[i], fp);

  if (! (sp || flag & TRUNC))
    for (i = 0; i < prec - 9; ++i)
      putc('0', fp);

  if (flag & LEFT)
    for (i = 0; i < w - n; ++i)
      putc(' ', fp);

  return max(w, n);
}

static int printgf(FILE *fp, float f, int w, int prec, int flag)
{
  int expo;

  if (prec < 0)
    prec = 6;
  if (prec == 0)
    prec = 1;

  normalize(f, &expo);

  if (-4 <= expo && expo < prec) {
    return printff(fp, f, w, prec - expo - 1, flag);
  } else {
    return printef(fp, f, w, prec - 1, flag);
  }
}
*/

static char *write_string_dst;
static int write_string(int c)
{
  *write_string_dst = c;
  ++write_string_dst;
  return c;
}

int printf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  return vfprintf(stdout, fmt, ap);
}

int fprintf(FILE *fp, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  return vfprintf(fp, fmt, ap);
}


int vprintf(const char *fmt, va_list ap)
{
  return vfprintf(stdout, fmt, ap);
}

int vfprintf(FILE *fp, const char *fmt, va_list ap)
{
  int ret = 0;

  if (! (fp->flag & _WRITE))
    return -1;

  for (; *fmt; ++fmt) {
    if (*fmt == '%') {
      int flag = 0;
      int alt = 0;
      int w = 0;
      int prec = -1;
      const char *bak = fmt++;
      if (*fmt == '%') {
        ++ret;
        putc('%', fp);
        continue;
      }
      while (strchr("-+ #0", *fmt)) {
        switch (*(fmt++)) {
          case '-': flag |= LEFT; break;
          case '+': flag |= PLUS; break;
          case ' ': flag |= SPACE; break;
          case '#': alt = -1; break;
          case '0': flag |= ZEROPAD; break;
        }
      }
      if (flag & LEFT)
        flag &= ~ZEROPAD;
      if (flag & PLUS)
        flag &= ~SPACE;
      if (*fmt == '*') {
        ++fmt;
        w = va_arg(ap, int);
      } else while (isdigit(*fmt)) {
        w = w * 10 + *(fmt++) - '0';
      }
      if (*fmt == '.') {
        ++fmt;
        prec = 0;
        if (*fmt == '*') {
          ++fmt;
          prec = va_arg(ap, int);
        } else while (isdigit(*fmt)) {
          prec = prec * 10 + *(fmt++) - '0';
        }
      }
      while (strchr("hlL", *fmt))
        ++fmt;
      switch (*fmt) {
        case 'c':
          ++ret;
          putc(va_arg(ap, int), fp);
          break;
        case 's':
          ret += prints(fp, va_arg(ap, char*), w, prec, flag);
          break;
        case 'd':
        case 'i':
          ret += printi(fp, va_arg(ap, int), 10, w, prec, flag | SIGN);
          break;
        case 'o':
          ret += printi(fp, va_arg(ap, int),  8, w, prec, flag | (alt & OCTPRE));
          break;
        case 'x':
          ret += printi(fp, va_arg(ap, int), 16, w, prec, flag | (alt & HEXPRE));
          break;
        case 'X':
          ret += printi(fp, va_arg(ap, int), 16, w, prec, flag | (alt & HEXPRE) | CAP);
          break;
        case 'u':
          ret += printi(fp, va_arg(ap, int), 10, w, prec, flag);
          break;
        case 'p':
          ret += printi(fp, va_arg(ap, int), 16, w, prec, flag | HEXPRE);
          break;
        // case 'e':
        //   ret += printef(fp, va_arg(ap, double), w, prec, flag | (alt & DOT));
        //   break;
        // case 'E':
        //   ret += printef(fp, va_arg(ap, double), w, prec, flag | (alt & DOT) | CAP);
        //   break;
        // case 'f':
        //   ret += printff(fp, va_arg(ap, double), w, prec, flag | (alt & DOT));
        //   break;
        // case 'F':
        //   ret += printff(fp, va_arg(ap, double), w, prec, flag | (alt & DOT) | CAP);
        //   break;
        // case 'g':
        //   ret += printgf(fp, va_arg(ap, double), w, prec, flag | (~alt & TRUNC));
        //   break;
        // case 'G':
        //   ret += printgf(fp, va_arg(ap, double), w, prec, flag | (~alt & TRUNC) | CAP);
        //   break;
        case 'n':
          *va_arg(ap, int*) = ret;
          break;
        default:
          ret += fmt - bak;
          for (; bak < fmt; ++bak){
            putc(*bak, fp);
          }
          --fmt;
          break;
      }
    } else {
      ++ret;
      putc(*fmt, fp);
    }
  }

  return ret;
}



/* stdlib.h */

void exit(int status)
{
  fflush(NULL);
  _exit();
}


void abort(void)
{
  fprintf(stderr, "abort!\n");
  exit(1);
}

int abs(int n)
{
  __asm("\
  mov r1, [rbp + 4] \n\
  sar r2, r1, 31    \n\
  xor r1, r1, r2    \n\
  sub r1, r1, r2    \n\
  ret               \n\
");
}


long strtol(const char *str, char **end, int base)
{
  long l = 0;

  if (base > 36) {
    if (end)
      *end = (char *)str;
    return 0;
  }

  if (base == 0) {
    if (*str == '0') {
      ++str;
      if (*str == 'x' || *str == 'X') {
        ++str;
        base = 16;
      } else {
        base = 8;
      }
    } else {
      base = 10;
    }
  }

  while (1) {
    char c = *str;
    long n = base;

    if (isdigit(c)) n = c - '0';
    if (islower(c)) n = c - 'a' + 10;
    if (isupper(c)) n = c - 'A' + 10;

    if (base <= n)
      break;

    ++str;
    l = l * base + n;
  }

  if (end)
    *end = (char *)str;
  return l;
}


static unsigned rbuf[32], ridx = -1;

int rand(void)
{
  if (ridx >= 32) srand(1);
  if (ridx == 31) {
    ridx = 0;
    return (rbuf[31] = rbuf[0] + rbuf[28]) >> 1;
  } else {
    int tmp = rbuf[ridx + 1] + rbuf[ridx + (ridx < 3 ? 29 : -3)];
    return (rbuf[ridx++] = tmp) >> 1;
  }
}

void srand(unsigned seed)
{
  int i;
  unsigned tmp, lo, hi;
  const unsigned mod = 0x7fffffff;
  ridx = 2;
  rbuf[0] = seed ? seed : 1;
  for (i = 1; i < 31; ++i) {
    /* rbuf[i] = (16807ll * (int)rbuf[i - 1] % mod + mod) % mod; */
    tmp = rbuf[i - 1];
    if ((int)tmp < 0) tmp = -tmp;
    lo = 16807 * (tmp & 0xffff);
    hi = 16807 * (tmp >> 16);
    tmp = (lo + ((hi & 0x7fff) << 16) + (hi >> 15));
    if (tmp >= mod) tmp -= mod;
    rbuf[i] = (int)rbuf[i - 1] < 0 && tmp ? mod - tmp : tmp;
  }
  rbuf[31] = rbuf[0]; rbuf[0] = rbuf[1]; rbuf[1] = rbuf[2];
  for (i = 34; i < 344; ++i) rand();
}


#define NALLOC 1024

struct header {
  struct header *next;
  size_t size;
};

typedef struct header Header;

static Header base;
static Header *freep;

void free(void *ap)
{
  Header *bp, *p;

  if (! ap)
    return;

  bp = (Header *)ap - 1;
  for (p = freep; !(p < bp && bp < p->next); p = p->next)
    if (p->next <= p && (p < bp || bp < p->next))
      break;

  if (bp + bp->size == p->next) {
    bp->size += p->next->size;
    bp->next = p->next->next;
  } else {
    bp->next = p->next;
  }

  if (p + p->size == bp) {
    p->size += bp->size;
    p->next = bp->next;
  } else {
    p->next = bp;
  }

  freep = p;
}

static Header *morecore(size_t nunits)
{
  Header *p;

  nunits = (nunits + NALLOC - 1) / NALLOC * NALLOC;
  p = (Header *)sbrk(nunits * sizeof(Header));
  if (p == (Header *)-1)
    return NULL;
  p->size = nunits;
  free(p + 1);
  return freep;
}

void *malloc(size_t nbytes)
{
  Header *p, *prevp;
  size_t nunits;

  nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
  if (! (prevp = freep)) {  /* no free list yet */
    base.next = freep = prevp = &base;
    base.size = 0;
  }

  for (p = prevp->next; ; prevp = p, p = p->next) {
    if (p->size >= nunits) {  /* big enough */
      if (p->size == nunits) {  /* exactly */
        prevp->next = p->next;
      } else {  /* allocate tail end */
        p->size -= nunits;
        p += p->size;
        p->size = nunits;
      }
      freep = prevp;
      return (void *)(p + 1);
    }
    if (p == freep)
      if (! (p = morecore(nunits)))
        return NULL;
  }

}

static size_t malloc_size(void *ap)
{
  Header *p;

  p = (Header *)ap - 1;
  return (p->size - 1) * sizeof(Header);
}

void *realloc(void *ptr, size_t size)
{
  void *new;

  if (! ptr)
    return malloc(size);

  if (size <= malloc_size(ptr))
    return ptr;

  new = malloc(size);
  if (! new)
    return NULL;

  memcpy(new, ptr, malloc_size(ptr));
  free(ptr);

  return new;
}

void *calloc(size_t n, size_t size)
{
  char *ptr = malloc(n * size);

  if (! ptr)
    return NULL;

  memset(ptr, 0, n * size);
  return (void *) ptr;
}


int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}


/* string.h */

size_t strlen(const char *str)
{
  size_t len = 0;

  while (*str++) {
    ++len;
  }
  return len;
}

int strcmp(const char *l, const char *r)
{
  while (*l) {
    if (*l++ != *r++) {
      return l[-1] < r[-1] ? -1 : 1;
    }
  }
  return *r ? -1 : 0;
}

int strncmp(const char *l, const char *r, size_t n)
{
  while (*l && 0 < n) {
    if (*l++ != *r++) {
      return l[-1] < r[-1] ? -1 : 1;
    }
    --n;
  }
  return 0;
}

char *strchr(const char *str, int c)
{
  do {
    if (*str == c) {
      return (char *)str;
    }
  } while (*str++);
  return NULL;
}

char *strcpy(char *dst, const char *src)
{
  char *d = dst;

  while (*d++ = *src++);
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
  char *d = dst;

  while (n-- > 0 && (*d++ = *src++));
  return dst;
}

void *memset(void *dst, int c, size_t n)
{
  char *d = dst;

  while (n-- > 0) {
    *d++ = c;
  }
  return dst;
}

void *memcpy(void *dst, const void *src, size_t n)
{
  const char *s = src;
  char *d = dst;

  while (n-- > 0) {
    *d++ = *s++;
  }
  return dst;
}

void*
memmove(void *dst, const void *src, size_t n)
{
  const char *s;
  char *d;

  s = src;
  d = dst;
  if(s < d && s + n > d){
    s += n;
    d += n;
    while(n-- > 0)
      *--d = *--s;
  } else
    while(n-- > 0)
      *d++ = *s++;

  return dst;
}


/* termios.h */

int tcgetattr(int fd, struct termios *termios_p)
{
  return ioctl(fd, TCGETA, termios_p);
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
  return ioctl(fd, TCSETA, termios_p);
}

void cfmakeraw(struct termios *termios_p)
{
  // Ignore optional_actions
  termios_p->c_lflag = 0;
}
