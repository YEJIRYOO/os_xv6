// user/priority_scheduler_test.c
#include "types.h"
#include "stat.h"
#include "user.h"

// user.h에 선언이 있어야 합니다.
int set_proc_priority(int pid, int prio);
int get_proc_priority(int pid);

static void burn_forever(void){
  volatile int x = 0;
  for(;;) x++;
}

int
main(int argc, char **argv)
{
  // 인자: [children(포함해서 총 N개, 기본 12)] [stopped_prio(기본 8)] [infinite_prio(기본 10)]
  int N = 12;
  int pr_stopped = 8;
  int pr_infinite = 10;

  if(argc >= 2) N = atoi(argv[1]);
  if(argc >= 3) pr_stopped = atoi(argv[2]);
  if(argc >= 4) pr_infinite = atoi(argv[3]);

  if(N < 2) N = 2;            // 최소 2개: 무한 1개 + 즉시 종료 1개 이상

  int i;
  int inf_pid = -1;

  for(i = 0; i < N; i++){
    int rc = fork();
    if(rc < 0){
      printf(1, "fork failed\n");
      exit();
    }
    if(rc == 0){
      // --- child ---
      int me = getpid();
      if(i == 0){
        // 무한 루프 자식 (가장 높은 우선순위)
        set_proc_priority(me, pr_infinite);
        printf(1, "PID(%d) : priority (%d) infinite ===\n", me, get_proc_priority(me));
        burn_forever();      // 종료 안 함
        // exit();           // 도달하지 않음
      }else{
        // 즉시 종료하는 자식들
        set_proc_priority(me, pr_stopped);
        printf(1, "PID(%d) : priority (%d) stopped\n", me, get_proc_priority(me));
        exit();
      }
    }else{
      // --- parent ---
      if(i == 0) inf_pid = rc;   // 무한 루프 자식 pid 기억
    }
  }

  // 부모: 종료한 자식들 수집/출력 (무한 루프 자식 1개 빼고 N-1개)
  for(i = 0; i < N - 1; i++){
    int done = wait();
    if(done > 0){
      printf(1, "Parent: Child PID(%d) has finished.\n", done);
    }
  }

  // 무한 루프 자식 종료 처리
  if(inf_pid > 0){
    kill(inf_pid);
    wait();
  }

  exit();
}
