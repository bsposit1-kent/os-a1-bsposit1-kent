#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
 
static void
usage(void)
{
  fprintf(2, "usage: sleep <ticks>\n");
  exit(1);
}
 
int
main(int argc, char *argv[])
{
  if(argc != 2)
    usage();
  char *s = argv[1];
  if(*s == 0)
    usage();
  for(int i = 0; s[i]; i++)
    if(s[i] < '0' || s[i] > '9')
      usage();
  pause(atoi(s));
  exit(0);
}
