#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <xv6/user.h>
#include <xv6/fs.h>

int
inum(char *path)
{
  struct stat st;
  int fd;

  fd = open(path, O_RDONLY);
  if(fstat(fd, &st) < 0){
    printf(2, "pwd: cannot stat %s\n", path);
    exit();
  }
  close(fd);
  return st.ino;
}

void
dirent_by_inum(char *dir_path, int inum, struct dirent *res)
{
  struct dirent de;
  int dir;

  dir = open(dir_path, O_RDONLY);
  while(read(dir, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == inum){
      break;
    }
  }
  close(dir);
  *res = de;
}

void
strprepend(char *dest, char *src, int n)
{
  memmove(dest + strlen(src), dest, n - strlen(src));
  memmove(dest, src, strlen(src));
}

int
main(int argc, char *argv[])
{
  char cur[256] = ".", parent[256] = "..";
  char res[256] = "";

  while(inum(cur) != inum(parent)){
    struct dirent cur_dirent;
    dirent_by_inum(parent, inum(cur), &cur_dirent);
    strprepend(res, cur_dirent.name, sizeof(res));
    strprepend(res, "/", sizeof(res));

    strcpy(cur, parent);
    strprepend(parent, "../", sizeof(parent));
  }
  if(res[0] == '\0'){
    strprepend(res, "/", sizeof(res));
  }
  printf(0, "%s\n", res);

  exit();
}
