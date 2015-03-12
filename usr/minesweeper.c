/*
 *
 * [TODO]
 *  : invalid command when reading "o 2"
 */

#include <sys/types.h>
#include <xv6/user.h>

#define MINESWEEPER_DEBUG 0

#define MAX_HEIGHT_SIZE 16
#define MAX_WIDTH_SIZE  30

// cell status
#define BOMB_MASK 15
#define OPENED 16
#define MARKED 32
#define BOMB   64

// command
#define INIT 0
#define IOPEN 1
#define OPEN 2
#define MARK 3
#define QUIT 4
#define HELP 5
#define NOP  6

// difficulty
#define EASY     1
#define MEDIUM   2
//#define ADVANCED 3

struct command{
  int com;
  int x, y;
} command;

char board[MAX_HEIGHT_SIZE][MAX_WIDTH_SIZE];
int height, width;
int bomb;
int initialized;

char buf[128];

char displayed[] = {
  // init, open, mark, bomb
     ' ',  '+',  'x',  'b' 
};

int board2disp(int i, int j){
  if((board[i][j] & OPENED) && (board[i][j] & BOMB)) return 3;
  if(board[i][j] & OPENED) return 1;
  if(board[i][j] & MARKED) return 2;
  return 0;
}

void print_2d(int n){
  if(n < 10){
    printf(1, " %d", n);  
  }else{
    printf(1, "%d", n);  
  }
}

void showboard(){
  int i,j;

  printf(1,"\033[2J");
  printf(1,"\033[1;1H");
  printf(1,"   ||");
  for(i=0; i<width; i++){
    print_2d(i);
    printf(1, " |");
  }
  printf(1,"\n---++");
  for(i=0; i<width; i++){
    printf(1,"---+", i);  
  }
  printf(1,"\n");
  for(i=0; i<width; i++){
    print_2d(i);
    printf(1, " || ");
    for(j=0; j<height; j++){
      if(board2disp(i,j) == 1)
        printf(1,"%d | ", board[i][j] & BOMB_MASK);
      else
        printf(1,"%c | ", displayed[board2disp(i,j)]);
    }
    printf(1,"\n");
  }
}

#if MINESWEEPER_DEBUG == 1
void dumpboard(){
  int i,j;
  for(i=0; i<width; i++){
    for(j=0; j<height; j++){
      print_2d(board[i][j]);
      printf(1, " ");
    } 
    printf(1,"\n");
  }
}
#endif

int myrand(int n) {
  static int lcg = 0;
  int x;
  lcg = 1664525 * lcg + 1013904223;
  x = lcg % n;
  return x < 0 ? x + n : x;
}

unsigned int xor128(void) { 
  static unsigned int x = 123456789;
  static unsigned int y = 362436069;
  static unsigned int z = 521288629;
  static unsigned int w = 88675123; 
  unsigned int t;
 
  t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
}

int isvalid(int x, int y){
  return 0 <= x && x < width && 0 <= y && y < height;
}

// set board size by nums
void init00(int h, int w, int b){
   height = h;
   width  = w;
   bomb   = b;
}

// set board size by macro
void init0(int s){
  switch(s){
  case EASY:
    init00(9, 9,10);
    return;
  case MEDIUM:
    init00(16,16,40);
    return;
//  case ADVANCED:
//    init00(30,16,99);
//    return;
  }  
}

// set number of bomb 0
void init1(){
  int i,j;
  for(i=0; i<width; i++){
    for(j=0; j<height; j++){
      board[i][j] = 0;  
    }  
  }
  initialized = 1;
}

// add number of bomb by bomb_number
void init2(int p, int q){
  int x, y;
  int i,j, bnum;
  int bomb_number;
  int drc[] = {-1, 0, 1};

  bomb_number = bomb;
  if(height*width < bomb_number-1) return;

  while(bomb_number > 0){
    x = xor128()%height;
    y = xor128()%width;

    if(p-1 <= x && x <= p+1 && q-1 <= y && y <= q+1) continue;
    if(board[x][y] & BOMB) continue;
    
    board[x][y] = BOMB;
    bomb_number--;

    for(i=0; i<3; i++){
      for(j=0; j<3; j++){
        if(i == 1 && j == 1) continue;

        if(isvalid(x+drc[i], y+drc[j])){
          bnum = board[x+drc[i]][y+drc[j]] & BOMB_MASK;
          bnum++;
          board[x+drc[i]][y+drc[j]] = (board[x+drc[i]][y+drc[j]] & ~BOMB_MASK) | bnum; 
        }
      }  
    }
  }
}

int isclear(){
  int i,j;

  for(i=0; i<width; i++){
    for(j=0; j<height; j++){
      if(!(board[i][j] & BOMB) && !(board[i][j] & OPENED)){
        return 0;
      }
    }  
  }
  return 1;
}

int isgameover(){
  int i,j;

  for(i=0; i<width; i++){
    for(j=0; j<height; j++){
      if((board[i][j] & BOMB) && (board[i][j] & OPENED)){
        return 1;  
      }  
    }  
  }
  return 0;
}

void getcom(struct command *com){
  char *c, *x, *y;
  int xint, yint;

  gets(buf, sizeof(buf));

  c = buf;
  while(*c == ' ' && *c != '\0') c++;
  x = c + 1;
  while(*x == ' ' && *x != '\0') x++;
  y = x;
  while('0' <= *y && *y <= '9')  y++;
  while(*y == ' ' && *y != '\0') y++;

  xint = 0;
  yint = 0;
  while('0' <= *x && *x <= '9'){
    xint = xint*10 + (*x - '0');
    x++;
  }
  while('0' <= *y && *y <= '9'){
    yint = yint*10 + (*y - '0');
    y++;
  }

  switch(*c){
  case 'i':
    com->com = INIT;
    com->x   = xint;
    return;
  case 'o':
    com->com = initialized == 1 ? IOPEN : OPEN;
    com->x   = xint;
    com->y   = yint;
    initialized = 0;
    return;
  case 'm':
    com->com = MARK;
    com->x   = xint;
    com->y   = yint;
    return;
  case 'q':
    com->com = QUIT;
    return;
  case 'h':
    com->com = HELP;
    return;
  default:
    com->com = NOP;
    return;
  }
}

void mine_open(struct command *com){
  int x, y;
  int i, j;
  struct command com_tmp;
  int drc[] = {-1, 0, 1};

  x = com->x;
  y = com->y;

  if(!(board[x][y] & (OPENED | MARKED)))
    board[x][y] |= OPENED;
  else 
    return;

  if((board[x][y] & BOMB_MASK) == 0){
    for(i=0; i<3; i++){
      for(j=0; j<3; j++){
        com_tmp.x = x+drc[i];
        com_tmp.y = y+drc[j];
        if(!(i==1 && j==1) && isvalid(com_tmp.x, com_tmp.y))
          mine_open(&com_tmp);
      }
    }
  }
}

void mine_mark(struct command *com){
  int x, y;
  x = com->x;
  y = com->y;

  if(!(board[x][y] & OPEN))
    board[x][y] ^= MARKED;
}

void mine_help(){
  printf(1, "commands:\n");
  printf(1, "  i, init x (x = 1,2)\n");
  printf(1, "  o, open x y\n");
  printf(1, "  m, mark x y\n");
  printf(1, "  q, qiut    \n");
}

int action(struct command *com){
  switch(com->com){
  case QUIT:
    return 1;
  case INIT:
    return 0;
  case IOPEN:
    init2(com->x, com->y);
    mine_open(com);
    return 0;
  case OPEN:
    mine_open(com);
    return 0;
  case MARK:
    mine_mark(com);
    return 0;
  default:
    printf(1,"program mistake: unknown action\n");
    return 1;
  }
}

int main(){
  int exit_game;
  struct command com;

  init0(EASY);
  init1();

  while(1){
    showboard();
#if MINESWEEPER_DEBUG == 1
    dumpboard();
#endif
    
    printf(1,"type 'help' to see available commands\n");
    com.com = NOP;
    while(com.com == NOP || com.com == HELP){
      if(com.com == HELP)
        mine_help();

      printf(1,"command> ");
      getcom(&com);
    }
 
    if(com.com == INIT){
      printf(1, "initinit!\n");
      if(EASY <= com.x && com.x <= MEDIUM)
        init0(com.x);
      else 
        init0(EASY);

      init1();
      continue;
    }

    exit_game = action(&com);

    if(exit_game){
      showboard();
      printf(1,"exit minesweeper\n");
      break;
    }
    if(isgameover()){
      showboard();
      printf(1,"gameover...\n");
      break;
    }
    if(isclear()){
      showboard();
      printf(1,"clear!\n");
      break;
    }
  }

  exit();
}
