#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int mypid = getpid();
  printf(1, "pid: %d , Initial priority: %d\n", mypid, get_proc_priority(mypid));

  int rc = fork();
  if(rc < 0){
    printf(1, "fork failed\n");
    exit();
  }

  if(rc == 0){
    // child
    int cpid = getpid();
    printf(1, "child pid: %d, child priority: %d\n", cpid, get_proc_priority(cpid));
    exit();
  }else{
    // parent
    int ppid = getpid();
    printf(1, "parent pid: %d, parent priority: %d\n", ppid, get_proc_priority(ppid));
    wait();
    exit();
  }
}
