#ifndef PARSE_H
#define PARSE_H
struct buf {
  int  cur;
  char str[256];
};
typedef struct buf buffer;

void skip_space_and_comma(buffer *);
int  get_token(char *, buffer *);
int  get_line (char *, buffer *);
int  regnum(char *);
unsigned read_num(char *);
void rm_comment(char *);
void rstrip(char *);
#endif
