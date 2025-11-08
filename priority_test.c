#include "types.h"
#include "user.h"

// user.h에 다음 프로토타입이 있어야 합니다.
// int set_proc_priority(int pid, int prio);
// int get_proc_priority(int pid);

int
main(int argc, char *argv[])
{
  int parent_prio = 10;
  int child_prio  = 1;

  if (argc >= 2) parent_prio = atoi(argv[1]);
  if (argc >= 3) child_prio  = atoi(argv[2]);

  // 부모: 초기 상태
  int mypid = getpid();
  printf(1, "pid: %d , Initial priority: %d\n", mypid, get_proc_priority(mypid));

  // 부모: 우선순위 적용
  set_proc_priority(mypid, parent_prio);
  printf(1, "pid: %d , priority: %d\n", mypid, get_proc_priority(mypid));

  // fork
  int rc = fork();
  if (rc < 0) {
    printf(1, "fork failed\n");
    exit();
  }

  if (rc == 0) {
    // ===== 자식 프로세스 =====
    int cpid = getpid();
    set_proc_priority(cpid, child_prio);  // 자식 우선순위 적용
    printf(1, "child pid: %d, child priority: %d\n", cpid, get_proc_priority(cpid));

    // CPU를 좀 쓰게 루프 (스케줄링 관찰용)
    for (;;) ;
    // exit();  // 도달하지 않음
  } else {
    // ===== 부모 프로세스 =====
    int ppid = getpid();
    printf(1, "parent pid: %d, parent priority: %d\n", ppid, get_proc_priority(ppid));

    // 잠시 대기 후 정리
    sleep(50);
    kill(rc);
    wait();
    exit();
  }
}
