#define OK  0
#define ERR 1

// console size
#define COLS  20
#define LINES 13

typedef struct charcell{
  char letter;
  char color;
} charcell;

// initializes
void initscr();

// outputs
int clear();                   // clear & refresh screen
int erase();                   // clear screen
int move(int, int);            // move cursol
int addch(char);               // print char @cursol
int addstr(char*);             // print string @culsol
int mvaddch(int, int, char);   // move & addch
int mvaddstr(int, int, char*); // move & addstr
int refresh();                 // refresh screen
