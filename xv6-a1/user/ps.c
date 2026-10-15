#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/uproc.h"
#include "user/user.h"

// Matches enum procstate in kernel/proc.h.
static char *states[] = {
  "unused", "used", "sleeping", "runnable", "running", "zombie"
};

static struct uproc procs[NPROC];   // static: keep the 4 KB user stack small

int
main(void)
{
  int n = getprocs(procs, NPROC);
  if(n < 0){
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }
  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");
  for(int i = 0; i < n; i++){
    int s = procs[i].state;
    char *name = (s >= 0 && s < 6) ? states[s] : "?";
    printf("%d\t%d\t%s\t%lu\t%s\n", procs[i].pid, procs[i].ppid, name,
           (uint64)procs[i].sz, procs[i].name);
  }
  exit(0);
}