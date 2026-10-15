#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static int
match(char *p, char *s)
{
  if(*p == 0)
    return *s == 0;
  if(*p == '*'){
    for(;;){
      if(match(p + 1, s))
        return 1;
      if(*s == 0)
        return 0;
      s++;
    }
  }
  if(*s == 0 || *p != *s)
    return 0;
  return match(p + 1, s + 1);
}

static char path[MAXPATH];
static char *pat;
static int want; 

static void
walk(int len)
{
  struct dirent de;
  struct stat st;
  char name[DIRSIZ + 1];
 
  for(int idx = 0;; idx++){
    int fd = open(path, O_RDONLY);
    if(fd < 0)
      return;
    int ok = 1;
    for(int k = 0; k <= idx; k++){
      if(read(fd, &de, sizeof(de)) != sizeof(de)){
        ok = 0;
        break;
      }
    }
    close(fd);
    if(!ok)
      return;
    if(de.inum == 0)
      continue;
 
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = 0;
    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;
 
    int nl = strlen(name);
    if(len + 1 + nl >= MAXPATH)
      continue;
    int base = len;
    if(base > 0 && path[base - 1] != '/')
      path[base++] = '/';
    memmove(path + base, name, nl + 1);
    int n = base + nl;
 
    if(stat(path, &st) == 0){
      int isdir = (st.type == T_DIR);
      if(match(pat, name) && (want == 0 || (want == 'd') == isdir))
        printf("%s\n", path);
      if(isdir)
        walk(n);
    }
    path[len] = 0;
  }
}
 
static void
usage(void)
{
  fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
  exit(1);
}
 
int
main(int argc, char *argv[])
{
  if(argc != 3 && argc != 5)
    usage();
  if(argc == 5){
    if(strcmp(argv[3], "-type") != 0)
      usage();
    if(strcmp(argv[4], "f") == 0)
      want = 'f';
    else if(strcmp(argv[4], "d") == 0)
      want = 'd';
    else
      usage();
  }
  pat = argv[2];
 
  struct stat st;
  int fd = open(argv[1], O_RDONLY);
  if(fd < 0 || strlen(argv[1]) >= MAXPATH){
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
  }
  int r = fstat(fd, &st);
  close(fd);
  if(r < 0){
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
  }
  if(st.type == T_DIR){
    strcpy(path, argv[1]);
    walk(strlen(path));
  }
  exit(0);
}