#include <termios.h>
#include <curses.h>

// termios
struct termios termios;

// screen buffer
typedef charcell ScreenType[LINES][COLS];
ScreenType screen;

// cursol location
int cursolX, cursolY;

// initializes
void initscr(){
  clear();
  cursolX = 0;
  cursolY = 0;
  tcgetattr(0, &termios);
}

void noecho(){
  struct termios noecho;
  tcgetattr(0, &noecho);
  noecho.c_lflag &= ~ECHO;
  tcsetattr(0, TCSANOW, &noecho);
}

void curs_set(int a){
  switch(a){
  case 0: //
    printf("\033[?25l");
    break;
  case 1: // default
    printf("\033[?25h");
    break;
  }
  fflush(stdout);
}

void endwin(){
  curs_set(1);
  printf("\n");
  fflush(stdout);
  tcsetattr(0, TCSANOW, &termios);
}

// outputs
int clear(){
  if(erase() == ERR)
    return ERR;
  if(refresh() == ERR)
    return ERR;
  return OK;
}

int erase(){
  int i,j;
  printf("\033[2J"); // clear screen
  fflush(stdout);
  for(i=0; i<LINES; ++i){
    for(j=0; j<COLS; ++j){
      screen[i][j].letter = ' ';
      screen[i][j].color = 0;
    }
  }
  return OK;
}

int move(int y, int x){
  if(y < 0 || LINES <= y)
    return ERR;
  if(x < 0 || COLS <= x)
    return ERR;

  cursolY = y;
  cursolX = x;
  return OK;
}

int addch(char c){
  screen[cursolY][cursolX].letter = c;
  cursolX++;
  if(cursolX >= COLS){
    cursolX -= COLS;
    cursolY++;
  }
  return OK;
}

int addstr(char*str){
  int i;
  for(i=0; str[i]!='\0'; i++){
    if(cursolY >= LINES)
      return ERR;

    screen[cursolY][cursolX].letter = str[i];

    cursolX++;
    if(cursolX >= COLS){
      cursolX -= COLS;
      cursolY++;
    }

  }
  return OK;
}

int mvaddch(int y, int x, char c){
  if(move(y,x) == ERR)
    return ERR;
  if(addch(c) == ERR)
    return ERR;
  return OK;
}

int mvaddstr(int y, int x, char*str){
  if(move(y,x) == ERR)
    return ERR;
  if(addstr(str) == ERR)
    return ERR;
  return OK;
}

int refresh(){
  int i,j;
  char buf[LINES*(COLS+1)];
  printf("\033[H"); // move cursol to top left
  fflush(stdout);
  for(i=0; i<LINES; ++i){
    for(j=0; j<COLS; ++j){
      buf[i*(COLS+1) + j] = screen[i][j].letter;
    }
    if(i!=LINES-1)
      buf[(i+1)*(COLS+1) -1] = '\n';
  }
  buf[LINES * (COLS+1) -1] = '\0';
  printf("%s", buf);
  fflush(stdout);
  return OK;
}
