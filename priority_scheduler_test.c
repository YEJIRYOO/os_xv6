// user/prio_sched_test.c
#include "types.h"
#include "stat.h"
#include "user.h"

// 프로토타입 (user.h에 이미 있으면 생략 가능)
int set_proc_priority(int pid, int prio);
int get_proc_priority(int pid);
int get_proc_runcnt(int pid);

static void burn(void) {
  volatile int x = 0;
  for(;;) x++;   // 타이머 틱마다 선점됨
}

int
main(int argc, char **argv)
{
  // 현재 구현은 "숫자 클수록 높은 우선순위" 가정
  int pr_hi = 9, pr_md = 5, pr_lo = 1;  // 필요하면 인자 받게 바꿔도 됨

  int c1 = fork();
  if(c1 == 0){
    set_proc_priority(getpid(), pr_hi);
    printf(1, "child-Hi pid=%d prio=%d\n", getpid(), get_proc_priority(getpid()));
    burn();
    exit();
  }

  int c2 = fork();
  if(c2 == 0){
    set_proc_priority(getpid(), pr_md);
    printf(1, "child-Md pid=%d prio=%d\n", getpid(), get_proc_priority(getpid()));
    burn();
    exit();
  }

  int c3 = fork();
  if(c3 == 0){
    set_proc_priority(getpid(), pr_lo);
    printf(1, "child-Lo pid=%d prio=%d\n", getpid(), get_proc_priority(getpid()));
    burn();
    exit();
  }

  // 부모: 주기적으로 실행횟수/우선순위 관측
  printf(1, "\nTick\tHi(runs,prio)\tMd(runs,prio)\tLo(runs,prio)\n");
  for(int t=0; t<=120; t++){            // 약 120틱 관찰
    if(t % 10 == 0){
      int r1 = get_proc_runcnt(c1);
      int r2 = get_proc_runcnt(c2);
      int r3 = get_proc_runcnt(c3);
      int p1 = get_proc_priority(c1);
      int p2 = get_proc_priority(c2);
      int p3 = get_proc_priority(c3);
      printf(1, "%3d\t%5d,%2d\t\t%5d,%2d\t\t%5d,%2d\n",
             t, r1, p1, r2, p2, r3, p3);
    }
    sleep(1);
  }

  // 종료
  kill(c1); kill(c2); kill(c3);
  wait(); wait(); wait();

  printf(1, "[prio_sched_test] done\n");
  exit();
}
