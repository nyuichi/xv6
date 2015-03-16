#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parse.h"

void
skip_space_and_comma(buffer *buf)
{
  char *s = buf->str;
  int  i = buf->cur;
  while (s[i] == ' ' || s[i] == ',') {
    ++i;
  }
  buf->cur = i;
}

int
get_token(char *token, buffer *buf)
{
  int i;
  char *s = buf->str;
  int n = strlen(buf->str);
  skip_space_and_comma(buf);
  i = buf->cur;

  if (i >= n) return -1;

  while (i < n &&
         s[i] != ',' &&
         s[i] != ' ' ) {
    ++i;
  }
  strncpy(token, &s[buf->cur], i - buf->cur);
  token[i - buf->cur]='\0';
  buf->cur = i;
  return 1;
}

int
get_line(char *token, buffer *buf)
{
  int i;
  char *s = buf->str;
  int n = strlen(buf->str);
  skip_space_and_comma(buf);
  i = buf->cur;

  if (i >= n) return -1;

  while (i < n &&
         s[i] != '\n') {
    ++i;
  }
  strncpy(token, &s[buf->cur], i - buf->cur);
  token[i - buf->cur]='\0';
  buf->cur = i;
  return 1;
}



int
regnum(char *r)
{
  if(r[0] != 'r') {
    fprintf(stderr, "%s is not register\n", r);
    return -1;
  }

  if (strcmp(r, "rsp") == 0) {
    return 30;
  }

  if (strcmp(r, "rbp") == 0) {
    return 31;
  }

  return atoi(r + 1);
}

unsigned
read_hex(char *hx)
{
  int i;
  unsigned r = 0;
  assert(hx[0] == '0' &&
         (hx[1] == 'x' || hx[1] == 'X'));
  i = 2;
  while(hx[i] != '\0') {
    unsigned t;
    if (isdigit(hx[i])) {
      t = hx[i] - '0';
    } else if (isalpha(hx[i])) {

      if (islower(hx[i])) {
        assert('a' <= hx[i] && hx[i] <= 'f');
        t = (hx[i] - 'a') + 10;
      } else {
        assert('A' <= hx[i] && hx[i] <= 'F');
        t = (hx[i] - 'A') + 10;
      }

    } else {
      fprintf(stderr, "Error: read_hex %s\n", hx);
      exit(1);
    }

    r = (r<<4) + t;
    ++i;
  }

  return r;

}


unsigned
read_num(char *num)
{
  if (num[0] == '0' && (num[1] == 'x' || num[1] == 'X'))
    return read_hex(num);
  return atoi(num);
}

void
rm_comment(char *str)
{
  int i;
  int l = strlen(str);

  for (i = 0; i < l; ++i) {
    if (str[i] == '#') {
      str[i] = '\0';
      break;
    }
  }

}

void
rstrip(char *str)
{
  int i = strlen(str) - 1;
  while(i >= 0) {
    if (str[i] != '\n' &&
        str[i] != '\r' &&
        str[i] != '\t' &&
        str[i] != ' ') {
      break;
    }
    --i;
  }
  str[i + 1] = '\0';
}
