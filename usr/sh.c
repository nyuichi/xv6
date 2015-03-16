// Shell.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10

struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd {
  int type;
  struct cmd *cmd;
  char *file;
  char *efile;
  int mode;
  int fd;
};

struct pipecmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd {
  int type;
  struct cmd *cmd;
};

int fork1(void);  // Fork but panics on failure.
void panic(char*);
struct cmd *parsecmd(char*);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2];
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    exit(0);

  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC:
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit(0);
    exec(ecmd->argv[0], ecmd->argv);
    fprintf(stderr, "exec %s failed\n", ecmd->argv[0]);
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    close(rcmd->fd);
    if(open(rcmd->file, rcmd->mode) < 0){
      fprintf(stderr, "open %s failed\n", rcmd->file);
      exit(1);
    }
    runcmd(rcmd->cmd);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    if(fork1() == 0)
      runcmd(lcmd->left);
    wait();
    runcmd(lcmd->right);
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    if(fork1() == 0){
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait();
    wait();
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    if(fork1() == 0)
      runcmd(bcmd->cmd);
    break;
  }
  exit(0);
}

char *readline();
void free_history();

int
main(void)
{
  char *buf;
  int fd;

  // Assumes three file descriptors open.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  // Read and run input commands.
  while((buf = readline())){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Clumsy but will have to do for now.
      // Chdir has no effect on the parent if run in the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(stderr, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait();
  }

  free_history();
  exit(0);
}

void
panic(char *s)
{
  fprintf(stderr, "%s\n", s);
  exit(1);
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

// Constructors

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->efile = efile;
  cmd->mode = mode;
  cmd->fd = fd;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd*)cmd;
}
// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if(*s == '>'){
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;

  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    fprintf(stderr, "leftovers: %s\n", s);
    panic("syntax");
  }
  nulterminate(cmd);
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parsepipe(ps, es);
  while(peek(ps, es, "&")){
    gettoken(ps, es, 0, 0);
    cmd = backcmd(cmd);
  }
  if(peek(ps, es, ";")){
    gettoken(ps, es, 0, 0);
    cmd = listcmd(cmd, parseline(ps, es));
  }
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a')
      panic("missing file for redirection");
    switch(tok){
    case '<':
      cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
      break;
    case '>':
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    case '+':  // >>
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    }
  }
  return cmd;
}

struct cmd*
parseblock(char **ps, char *es)
{
  struct cmd *cmd;

  if(!peek(ps, es, "("))
    panic("parseblock");
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);
  if(!peek(ps, es, ")"))
    panic("syntax - missing )");
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  if(peek(ps, es, "("))
    return parseblock(ps, es);

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|)&;")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a')
      panic("syntax");
    cmd->argv[argc] = q;
    cmd->eargv[argc] = eq;
    argc++;
    if(argc >= MAXARGS)
      panic("too many args");
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}

// NUL-terminate all the counted strings.
struct cmd*
nulterminate(struct cmd *cmd)
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    return 0;

  switch(cmd->type){
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    for(i=0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    nulterminate(rcmd->cmd);
    *rcmd->efile = 0;
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    nulterminate(pcmd->left);
    nulterminate(pcmd->right);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    nulterminate(lcmd->left);
    nulterminate(lcmd->right);
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}


/* readline library */

#define PROMPT      "$ "
#define PROMPT_LEN   2
#define HISTORY     16
#define TMP_UNIT    64
#define REALLOC(p, size) \
  do if (! (p = realloc(p, size))) { fprintf(stderr, "realloc failed\n"); exit(1); } while (0)

char *hist[HISTORY], *tmp;
int hist_index, cur_index;
int tmp_size;
int pos, len, col = 80;

void extend_tmp()
{
  tmp_size += TMP_UNIT;
  REALLOC(tmp, tmp_size + 1);
}

void move_cursor(int target, int opt)
{
  int cx = ((opt ? len : pos) + PROMPT_LEN) % col;
  int cy = ((opt ? len : pos) + PROMPT_LEN) / col;
  int tx = (target + PROMPT_LEN) % col;
  int ty = (target + PROMPT_LEN) / col;
  if (ty < cy) printf("\x1b[%dA", cy - ty);
  if (ty > cy) printf("\x1b[%dB", ty - cy);
  if (tx > cx) printf("\x1b[%dC", tx - cx);
  if (tx < cx) printf("\x1b[%dD", cx - tx);
  pos = target;
}

void key_left()  { if (pos > 0)   move_cursor(pos - 1, 0); }
void key_right() { if (pos < len) move_cursor(pos + 1, 0); }
void key_home()  { move_cursor(0, 0); }
void key_end()   { move_cursor(len, 0); }

void move_history(int idx)
{
  cur_index = idx;
  key_home();
  pos = len;
  memset(tmp, ' ', len);
  printf("%s", tmp);
  if ((len + PROMPT_LEN) % col == 0)
    printf("\n");
  key_home();
  if (idx == hist_index) {
    len = 0;
    tmp[0] = '\0';
    return;
  }
  pos = len = strlen(hist[idx]);
  if (tmp_size < len) {
    tmp_size = (len / TMP_UNIT + 1) * TMP_UNIT;
    REALLOC(tmp, tmp_size + 1);
  }
  memcpy(tmp, hist[idx], len + 1);
  printf("%s", tmp);
  if ((len + PROMPT_LEN) % col == 0)
    printf("\n");
}

void key_up()
{
  int idx = (cur_index - 1 + HISTORY) % HISTORY;
  if (idx == hist_index || hist[idx] == NULL)
    return;
  move_history(idx);
}

void key_down()
{
  if (cur_index == hist_index)
    return;
  move_history((cur_index + 1) % HISTORY);
}

void key_delete()
{
  int i;
  if (pos == len)
    return;
  for (i = pos; i < len; ++i)
    tmp[i] = tmp[i + 1];
  printf("%s ", tmp + pos);
  if ((len + PROMPT_LEN) % col == 0)
    printf("\n");
  move_cursor(pos, 1);
  --len;
}

void key_backspace()
{
  if (pos == 0)
    return;
  key_left();
  key_delete();
}

void key_enter()
{
  int prv;
  key_end();
  printf("\n");
  while (len > 0 && tmp[len - 1] == ' ')
    tmp[--len] = '\0';
  prv = (hist_index - 1 + HISTORY) % HISTORY;
  if (len == 0 || tmp[0] == ' ' || (hist[prv] && !strcmp(tmp, hist[prv])))
    return;
  REALLOC(hist[hist_index], len + 1);
  memcpy(hist[hist_index], tmp, len + 1);
  hist_index = (hist_index + 1) % HISTORY;
}

void insert_char(int c)
{
  int i;
  if (len == tmp_size)
    extend_tmp();
  for (i = len; i >= pos; --i)
    tmp[i + 1] = tmp[i];
  tmp[pos] = c;
  printf("%s", tmp + pos);
  ++pos; ++len;
  if ((len + PROMPT_LEN) % col == 0)
    printf("\n");
  move_cursor(pos, 1);
}

// returned pointer is valid until next readline call
char *readline()
{
  static int init_term = 0;
  static struct termios original, settings;

  int state = 0, ctrl_d = 0;

  tmp_size = TMP_UNIT;
  REALLOC(tmp, tmp_size + 1);

  printf(PROMPT);
  fflush(stdout);

  if (!init_term) {
    init_term = 1;
    tcgetattr(0, &original);
    settings = original;
    cfmakeraw(&settings);
  }
  tcsetattr(0, TCSANOW, &settings);

  tmp[0] = '\0';
  pos = len = 0;
  cur_index = hist_index;

  while (1) {
    char c;
    read(0, &c, 1);
    if (c == '\x1b') {
      state = 1;
    } else if (state == 1) {    // \x1b
      state = 0;
      if (c == '[') state = 2;
      if (c == 'O') state = 3;
    } else if (state == 2) {    // \x1b[
      switch (c) {
        case 'A': state = 0; key_up();    break;
        case 'B': state = 0; key_down();  break;
        case 'C': state = 0; key_right(); break;
        case 'D': state = 0; key_left();  break;
        case '1': state = 4; break;
        case '3': state = 5; break;
        case '4': state = 6; break;
        case '2': case '5': case '6': state = 7; break;
        default:  state = 0; break;
      }
    } else if (state == 3) {    // \x1bO
      state = 0;
      if (c == 'H') key_home();
      if (c == 'F') key_end();
    } else if (state == 4) {    // \x1b[1
      state = 0;
      if (c == '~') key_home();
      if (c == ';') state = 8;
    } else if (state == 5) {    // \x1b[3
      state = 0;
      if (c == '~') key_delete();
    } else if (state == 6) {    // \x1b[4
      state = 0;
      if (c == '~') key_end();
    } else if (state == 7) {
      state = 0;
    } else if (state == 8) {    // \x1b[1;
      state = c == '5' ? 9 : 0;
    } else if (state == 9) {    // \x1b[1;5
      state = 0;
      if (c == 'C') key_end();
      if (c == 'D') key_home();
    } else if (c == 127) {
      key_backspace();
    } else if (c < 32) {
      if (c == 10 || c == 13) { key_enter(); break; }
      if (c == 4) {
        if (! len) { ctrl_d = 1; break; }
        else key_delete();
      }
      if (c ==  1) key_home();
      if (c ==  2) key_left();
      if (c ==  5) key_end();
      if (c ==  6) key_right();
      if (c ==  8) key_backspace();
      if (c == 14) key_down();
      if (c == 16) key_up();
    } else {
      insert_char(c);
    }
    fflush(stdout);
  }

  tcsetattr(0, TCSANOW, &original);

  if (ctrl_d) {
    printf("\n");
    fflush(stdout);
    return NULL;
  }

  fflush(stdout);
  return tmp;
}

void free_history() {
  int i;
  for (i = 0; i < HISTORY; ++i)
    free(hist[i]);
  free(tmp);
}

