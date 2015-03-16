#include<string.h>
#include "ops.h"

static const char alu4_table[][8] = {
  "add", "sub",
  "shl", "shr", "sar",
  "and", "or", "xor",
  "cmpult", "cmpule",
  "cmpne", "cmpeq",
  "cmplt", "cmple"
};

static const int alu4_code[] = {
   0,  1,
   2,  3, 4,
   5,  6, 7,
  22, 23,
  24, 25,
  26, 27
};

static const char misc0_table[][12] = {
  "sysenter", "sysexit"
};

static const int misc0_code[] = {
  12, 13
};

static const char misc2_table[][8] = {
  "ldl", "jl"
};
static const int misc2_code[] = {
  2, 4
};

static const char misc3_table[][8] = {
  "ldh", "ld", "ldb", "st", "stb", "bne", "beq"
};

static const int misc3_code[] = {
  3, 6, 7, 8, 9, 14, 15
};

int find_misc0(char *op) {
  unsigned i;
  for(i = 0; i < sizeof(misc0_code)/sizeof(int); ++i) {
    if(strcmp(op, misc0_table[i]) == 0)
      return misc0_code[i];
  }
  return -1;
}


int find_alu4(char *op) {
  unsigned i;
  for(i = 0; i < sizeof(alu4_code)/sizeof(int); ++i) {
    if(strcmp(op, alu4_table[i]) == 0)
      return alu4_code[i];
  }
  return -1;
}

int find_misc2(char *op) {
  unsigned i;
  for(i = 0; i < sizeof(misc2_code)/sizeof(int); ++i) {
    if(strcmp(op, misc2_table[i]) == 0)
      return misc2_code[i];
  }
  return -1;
}

int find_misc3(char *op) {
  unsigned i;
  for(i = 0; i < sizeof(misc3_code)/sizeof(int); ++i) {
    if(strcmp(op, misc3_table[i]) == 0)
      return misc3_code[i];
  }
  return -1;
}
