#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
 
#define WLEN 128

static char *args[MAXARG];
static char words[MAXARG][WLEN];
static int nbase;

static int fail;
 
static void
usage(void)
{
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}
 
static void
run(int nextra)
{
  args[nbase + nextra] = 0;
  int pid = fork();
  if(pid < 0){
    fprintf(2, "xargs: fork failed\n");
    exit(1);
  }
  if(pid == 0){
    exec(args[0], args);
    fprintf(2, "xargs: exec %s failed\n", args[0]);
    exit(1);
  }
  int status;
  wait(&status);
  if(status != 0)
    fail = 1;
}
 
int
main(int argc, char *argv[])
{
  int i = 1, nflag = 0, N = 0;
 
  if(i < argc && strcmp(argv[i], "-n") == 0){
    if(i + 1 >= argc)
      usage();
    char *s = argv[i + 1];
    if(*s == 0)
      usage();
    for(int k = 0; s[k]; k++)
      if(s[k] < '0' || s[k] > '9')
        usage();
    N = atoi(s);
    if(N < 1)
      usage();
    nflag = 1;
    i += 2;
  }
  if(i >= argc)
    usage();
 
  for(; i < argc; i++){
    if(nbase >= MAXARG - 1){
      fprintf(2, "xargs: too many arguments\n");
      exit(1);
    }
    args[nbase++] = argv[i];
  }

  int cap = MAXARG - 1 - nbase;
  if(cap < 1){
    fprintf(2, "xargs: too many arguments\n");
    exit(1);
  }
  int limit = (nflag && N < cap) ? N : cap;
 
  char buf[512];
  int n = 0; 
  int wl = 0;
  int len;
 
  while((len = read(0, buf, sizeof(buf))) > 0){
    for(int k = 0; k < len; k++){
      char c = buf[k];
      if(c != ' ' && c != '\t' && c != '\n'){
        if(wl >= WLEN - 1){
          fprintf(2, "xargs: word too long\n");
          exit(1);
        }
        words[n][wl++] = c;
        continue;
      }
      if(wl > 0){
        words[n][wl] = 0;
        args[nbase + n] = words[n];
        n++;
        wl = 0;
        if(n == limit){
          run(n);
          n = 0;
        }
      }
      if(c == '\n' && !nflag && n > 0){
        run(n);
        n = 0;
      }
    }
  }
  if(wl > 0){
    words[n][wl] = 0;
    args[nbase + n] = words[n];
    n++;
  }
  if(n > 0)
    run(n);
  exit(fail);
}