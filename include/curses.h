#ifndef _CURSES_H
#define _CURSES_H

#include <stdio.h>

#define OK  0
#define ERR 1

// console size
#define COLS  80
#define LINES 24

typedef struct charcell{
  char letter;
  char color;
} charcell;

// initializes
void initscr();
void noecho();
void curs_set(int);
void endwin();

// outputs
int clear();                   // clear & refresh screen
int erase();                   // clear screen
int move(int, int);            // move cursol
int addch(char);               // print char @cursol
int addstr(char*);             // print string @culsol
int mvaddch(int, int, char);   // move & addch
int mvaddstr(int, int, char*); // move & addstr
int refresh();                 // refresh screen

#endif /* curses.h */
