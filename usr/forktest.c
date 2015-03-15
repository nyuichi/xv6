// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N  1000

void
forktest(void)
{
  int n, pid;

  printf("fork test\n");

  for(n=0; n<N; n++){
    pid = fork();
    if(pid < 0)
      break;
    if(pid == 0)
      exit(0);
  }

  if(n == N){
    printf("fork claimed to work N times!\n", N);
    exit(0);
  }

  for(; n > 0; n--){
    if(wait() < 0){
      printf("wait stopped early\n");
      exit(0);
    }
  }

  if(wait() != -1){
    printf("wait got too many\n");
    exit(0);
  }

  printf("fork test OK\n");
}

int
main(void)
{
  forktest();
  exit(0);
}
