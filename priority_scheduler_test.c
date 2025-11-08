// user/priority_scheduler_test.c
#include "types.h"
#include "stat.h"
#include "user.h"

// 커널에 이미 만들어 둔 시스템콜 프로토타입
int set_proc_priority(int pid, int prio);
int get_proc_priority(int pid);

// (선택) 실행 횟수 조회 시스템콜이 있다면 주석 해제하세요.
// int get_proc_runcnt(int pid);

static void cpu_burn(const char *tag) {
  // 너무 많이 찍히지 않게 큰 주기로만 한 글자 출력
  volatile unsigned long long x = 0;
  while(1){
    x++;
    if((x & ((1ULL<<24)-1)) == 0){ // 대략 주기적으로 한 글자
      printf(1, "%s", tag);
    }
  }
}

int
main(int argc, char **argv)
{
  // 현재 구현: 숫자가 클수록 높은 우선순위
  int pr_hi = 9, pr_md = 5, pr_lo = 1;

  // 필요하면 인자로 덮어쓰기: priority_scheduler_test [hi] [md] [lo]
  if(argc >= 2) pr_hi = atoi(argv[1]);
  if(argc >= 3) pr_md = atoi(argv[2]);
  if(argc >= 4) pr_lo = atoi(argv[3]);

  // 자식 1 (High)
  int c1 = fork();
  if(c1 == 0){
    int me = getpid();
    set_proc_priority(me, pr_hi);
    printf(1, "child-Hi pid=%d prio=%d\n", me, get_proc_priority(me));
    cpu_burn("[H]");
    exit();
  }

  // 자식 2 (Mid)
  int c2 = fork();
  if(c2 == 0){
    int me = getpid();
    set_proc_priority(me, pr_md);
    printf(1, "child-Md pid=%d prio=%d\n", me, get_proc_priority(me));
    cpu_burn("[M]");
    exit();
  }

  // 자식 3 (Low)
  int c3 = fork();
  if(c3 == 0){
    int me = getpid();
    set_proc_priority(me, pr_lo);
    printf(1, "child-Lo pid=%d prio=%d\n", me, get_proc_priority(me));
    cpu_burn("[L]");
    exit();
  }

  // 부모: 주기적으로 우선순위(그리고 원하면 실행횟수) 관찰
  printf(1, "\nTick\tHi(prio)\tMd(prio)\tLo(prio)\n");
  for(int t=0; t<=150; t++){
    if(t % 10 == 0){
      int p1 = get_proc_priority(c1);
      int p2 = get_proc_priority(c2);
      int p3 = get_proc_priority(c3);

      // (선택) 실행 횟수도 찍고 싶으면 아래 주석을 해제
      // int r1 = get_proc_runcnt(c1);
      // int r2 = get_proc_runcnt(c2);
      // int r3 = get_proc_runcnt(c3);
      // printf(1, "%3d\t%2d(%d)\t\t%2d(%d)\t\t%2d(%d)\n", t, p1, r1, p2, r2, p3, r3);

      printf(1, "%3d\t%2d\t\t%2d\t\t%2d\n", t, p1, p2, p3);
    }
    sleep(1);
  }

  // 정리
  kill(c1); kill(c2); kill(c3);
  wait(); wait(); wait();
  printf(1, "\n[priority_scheduler_test] done\n");
  exit();
}
