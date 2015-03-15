#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(stderr, "Usage: ln old new\n");
    exit(1);
  }
  if(link(argv[1], argv[2]) < 0)
    fprintf(stderr, "link %s %s: failed\n", argv[1], argv[2]);
  exit(0);
}
