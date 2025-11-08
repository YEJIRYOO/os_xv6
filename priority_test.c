#include "types.h"
#include "user.h"

int main(int argc, char *argv[]){
    int pid=getpid();
    int priority=get_proc_priority(pid);

    printf(1,"pid: %d , Initial priority: %d\n",pid,priority);

    priority=atoi(argv[1]);

    printf(1,"pid: %d , priority: %d\n",pid,get_proc_priority(pid));

    pid=fork();

    int p_pid=getpid();
    int p_priority=get_proc_priority(p_pid);

    int c_pid=getpid();
    int c_priority=get_proc_priority(c_pid);

    prtinf(1,"parent pid: %d, parent priority: %d\n",p_pid,p_priority);
    prtinf(1,"child pid: %d, child priority: %d\n",c_pid,c_priority);

   return 0;
}