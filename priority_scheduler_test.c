// user/priority_scheduler_test.c
#include "types.h"
#include "stat.h"
#include "user.h"

// 커널에 구현해둔 시스템콜 프로토타입 (user.h에도 선언돼 있어야 함)
int set_proc_priority(int pid, int prio);
int get_proc_priority(int pid);

// CPU를 꾸준히 소모하는 루프 (출력 없음: 콘솔 인터리빙 줄이기)
static void cpu_burn(void) {
  volatile unsigned long long x = 0;
  for(;;) x++;
}

// 간단한 2자리 패딩(폭 지정 미지원이라 수동 정렬용)
static void print2(int n){
  if(n < 10) printf(1, " %d", n);
  else       printf(1, "%d", n);
}

int
main(int argc, char **argv)
{
  // 현재 스케줄러는 "숫자가 클수록 높은 우선순위"
  int pr_hi = 9, pr_md = 5, pr_lo = 1;
  if(argc >= 2) pr_hi = atoi(argv[1]);
  if(argc >= 3) pr_md = atoi(argv[2]);
  if(argc >= 4) pr_lo = atoi(argv[3]);

  // 자식 1 (High)
  int c1 = fork();
  if(c1 == 0){
    int me = getpid();
    set_proc_priority(me, pr_hi);
    printf(1, "child-Hi pid=%d prio=%d\n", me, get_proc_priority(me));
    cpu_burn(); // never returns
    exit();
  }

  // 자식 2 (Mid)
  int c2 = fork();
  if(c2 == 0){
    int me = getpid();
    set_proc_priority(me, pr_md);
    printf(1, "child-Md pid=%d prio=%d\n", me, get_proc_priority(me));
    cpu_burn();
    exit();
  }

  // 자식 3 (Low)
  int c3 = fork();
  if(c3 == 0){
    int me = getpid();
    set_proc_priority(me, pr_lo);
    printf(1, "child-Lo pid=%d prio=%d\n", me, get_proc_priority(me));
    cpu_burn();
    exit();
  }

  // 부모: 주기적으로 각 자식의 현재 priority를 관찰
  printf(1, "\nTick\tHi(pr)\tMd(pr)\tLo(pr)\n");
  for(int t=0; t<=120; t++){
    if(t % 10 == 0){
      int p1 = get_proc_priority(c1);
      int p2 = get_proc_priority(c2);
      int p3 = get_proc_priority(c3);

      // 정렬: Tick은 그냥 %d, 나머지는 2자리 패딩
      printf(1, "%d\t", t);
      print2(p1); printf(1, "\t");
      print2(p2); printf(1, "\t");
      print2(p3); printf(1, "\n");
    }
    sleep(1); // 한 틱
  }

  // 종료 정리
  kill(c1); kill(c2); kill(c3);
  wait(); wait(); wait();

  printf(1, "\n[priority_scheduler_test] done\n");
  exit();
}
