// init: The initial user-level program

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

char *argv[] = { "sh", 0 };

int
main(void)
{
  int pid, wpid;

  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  mknod("min-rt", 2, 1);
  dup(0);  // stdout
  dup(0);  // stderr
  for(;;){
    printf("init: starting sh\n");

    pid = fork();
    if(pid < 0){
      fprintf(stderr, "init: fork failed\n");
      exit(1);
    }
    if(pid == 0){
      exec("sh", argv);
      fprintf(stderr, "init: exec sh failed\n");
      exit(1);
    }
    while((wpid=wait()) >= 0 && wpid != pid)
      printf("zombie!\n");
  }
}
