#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
  int i;

  for(i = 1; i < argc; i++)
    printf("%s%s", argv[i], i+1 < argc ? " " : "\n");
  exit(0);
}
