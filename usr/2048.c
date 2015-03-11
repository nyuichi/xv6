#include <sys/types.h>
#include <xv6/user.h>

#define true  1
#define false 0
typedef int bool;

#define UP    0
#define DOWN  1
#define RIGHT 2
#define LEFT  3
int dx[4] = {  0, 0, 1, -1};
int dy[4] = { -1, 1, 0,  0};

int board[4][4];
int score;

int myrand(int n) {
  static int lcg = 0;
  int x;
  lcg = 1664525 * lcg + 1013904223;
  x = lcg % n;
  return x < 0 ? x + n : x;
}
int getchar(void)
{
  char buf[1];

  read(1, buf, 1);

  return buf[0];
}

void print_int(int d)
{
  if (d < 10) {
    printf(1, "   %d", d);
  } else if (d < 100) {
    printf(1, "  %d", d);
  } else if (d < 1000) {
    printf(1, " %d", d);
  } else {
    printf(1, "%d", d);
  }
}

void showBoard(){
  int i,j;
  printf(1, "\033[2J");
  printf(1, "\033[1;1H");
  for(i=0; i<4; i++){
    printf(1, "+------+------+------+------+\n");
    printf(1, "|      |      |      |      |\n");
    printf(1, "| ");
    for(j=0; j<4; j++){
      if(board[i][j] != 0) {
        print_int(1 << board[i][j]);
        printf(1, " | ");
      } else {
        printf(1, "     | ");
      }
    }
    printf(1, "\n");
    printf(1, "|      |      |      |      |\n");
  }
  printf(1, "+------+------+------+------+\n");
}

bool isValid(int x, int y){
  if(x < 0 || 4 <= x)
    return false;
  if(y < 0 || 4 <= y)
    return false;
  return true;
}

bool movable(int dir){
  int i,j;
  int ay,ax;
  for(i=0; i<4; i++){
    for(j=0; j<4; j++){
      ay = i+dy[dir]; ax = j+dx[dir];
      if(isValid(ax, ay) && board[i][j]!=0){
        if(board[ay][ax]==0 || board[ay][ax]==board[i][j]){
          return true;
        }
      }
    }
  }
  return false;
}

bool gameover(){
  int i,j;
  for(i=0; i<4; i++){
    for(j=0; j<4; j++){
      if(board[i][j] == 11)
        return true;
    }
  }
  for(i=0; i<4; i++){
    if(movable(i))
      return false;
  }
  return true;
}

void putNum(){
  int n, b;
  if(myrand(10)==0)
    n=2;
  else
    n=1;

  do{
    b = myrand(16);
  } while(board[b%4][b/4] != 0);
  board[b%4][b/4] = n;
}

void init(){
  int i,j,b;
  for(i=0; i<4; i++)
    for(j=0; j<4; j++)
      board[i][j] = 0;

  score = 0;

  b = myrand(16);
  board[b%4][b/4] = 1;

  putNum();

  printf(1, "\033[2J");
}

int c2dir(char c){
  switch(c){
  case 'w':
    return 0;
  case 's':
    return 1;
  case 'd':
    return 2;
  case 'a':
    return 3;
  default:
    return -1;
  }
}

void push(int d){
  bool f;
  int i,j,ay,ax;
  do{
    f=false;
    for(i=0; i<4; i++){
      for(j=0; j<4; j++){
        ay=i+dy[d]; ax=j+dx[d];
        if(isValid(ax,ay) && board[ay][ax]==0 && board[i][j]!=0){
          board[ay][ax] = board[i][j];
          board[i][j] = 0;
          f=true;
        }
      }
    }
  } while(f);
}

void merge(int d){
  int i,j;
  switch(d){
  case UP:
    for(j=0;j<4;j++){
      for(i=0;i<3;){
        if(board[i][j]==board[i+1][j] && board[i][j]!=0){
          board[i][j]++;
          score += 1 << board[i][j];
          board[i+1][j]=0;
          i+=2;
        } else {
          i++;
        }
      }
    }
    break;
  case DOWN:
    for(j=0;j<4;j++){
      for(i=3;i>0;){
        if(board[i][j]==board[i-1][j] && board[i][j]!=0){
          board[i][j]++;
          score += 1 << board[i][j];
          board[i-1][j]=0;
          i-=2;
        } else {
          i--;
        }
      }
    }
    break;
  case RIGHT:
    for(i=0;i<4;i++){
      for(j=3;j>0;){
        if(board[i][j]==board[i][j-1] && board[i][j]!=0){
          board[i][j]++;
          score += 1 << board[i][j];
          board[i][j-1]=0;
          j-=2;
        } else {
          j--;
        }
      }
    }
    break;
  case LEFT:
    for(i=0;i<4;i++){
      for(j=0;j<3;){
        if(board[i][j]==board[i][j+1] && board[i][j]!=0){
          board[i][j]++;
          score += 1 << board[i][j];
          board[i][j+1]=0;
          j+=2;
        } else {
          j++;
        }
      }
    }
    break;
  }
}

void Move(int d){
  push(d);
  merge(d);
  push(d);
}

int main(){
  int d;
  init();

  do{
    showBoard();
    printf(1, "score: %d\n", score);
    printf(1, "(enter \'q\' to exit...)\n", score);
    printf(1, "> ");
    do{
      char c;
      c = getchar();
      if (c == 'q')
        exit();
      d = c2dir(c);
    } while(d==-1 || !movable(d));
    Move(d);
    putNum();
  } while(!gameover());

  showBoard();
  printf(1, "gameover...\n");
  printf(1, "your score is: %d\n\n", score);

  return 0;
}
