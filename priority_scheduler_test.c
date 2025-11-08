// user/priority_scheduler_test.c
#include "types.h"
#include "stat.h"
#include "user.h"

// user.h에 선언되어 있어야 함
int set_proc_priority(int pid, int prio);
int get_proc_priority(int pid);

static void burn_forever(void){
  volatile int x = 0;
  for(;;) x++;
}

int
main(int argc, char **argv)
{
  // 인자: [children] [stopped_prio] [infinite_prio]
  int N = 12;          // 총 자식 수(무한 1 + 즉시종료 N-1)
  int pr_stopped = 8;  // 종료 자식 우선순위
  int pr_infinite = 10;// 무한 자식 우선순위

  if(argc >= 2) N = atoi(argv[1]);
  if(argc >= 3) pr_stopped = atoi(argv[2]);
  if(argc >= 4) pr_infinite = atoi(argv[3]);
  if(N < 2) N = 2;

  int i, inf_pid = -1;

  for(i = 0; i < N; i++){
    int rc = fork();
    if(rc < 0){
      printf(1, "fork failed\n");
      exit();
    }

    if(rc == 0){
      // ---- child ----
      int me = getpid();

      if(i == 0){
        // 무한 루프 자식 (높은 우선순위)
        set_proc_priority(me, pr_infinite);
        // 약간의 지연으로 헤더와 겹침 방지
        sleep((me & 7));
        printf(1, "PID(%d) : priority (%d) infinite ===\n",
               me, get_proc_priority(me));
        burn_forever(); // never returns
        exit(); // not reached
      }else{
        // 즉시 종료하는 자식들 (같은 우선순위)
        set_proc_priority(me, pr_stopped);
        // 서로 출력이 겹치지 않도록 pid 기반 짧은 지연
        sleep((me & 7));
        printf(1, "PID(%d) : priority (%d) stopped\n",
               me, get_proc_priority(me));
        exit();
      }
    }else{
      // ---- parent ----
      if(i == 0) inf_pid = rc;  // 무한 자식 PID 기록
    }
  }

  // 부모: 종료한 자식들 수집 (무한자식 제외 N-1개)
  for(i = 0; i < N - 1; i++){
    int done = wait();
    if(done > 0){
      printf(1, "Parent: Child PID(%d) has finished.\n", done);
    }
  }

  // 무한 자식 정리
  if(inf_pid > 0){
    kill(inf_pid);
    wait();
  }

  exit();
}
