#include "types.h"
#include "stat.h"
#include "user.h"

int set_proc_priority(int pid, int prio);
int get_proc_priority(int pid);
int uptime(void); // xv6 표준 시스템콜

// xv6 printf는 폭 지정 미지원 → 2자리 패딩 보조
static void print2(int n){
  if(n < 10) printf(1, " %d", n);
  else       printf(1, "%d", n);
}

// 주어진 윈도우(틱) 동안 "실행 이벤트"를 근사 측정
// 원리: 루프를 돌면서 uptime()의 값이 바뀔 때마다 1 증가
// -> 프로세스가 실제로 실행 중일 때만 경계 변화를 자주 관측함
static int measure_run_events(int window_ticks){
  int end = uptime() + window_ticks;
  int last = uptime();
  int cnt = 0;
  // 바쁜 루프로 틱 경계 감시
  while(uptime() < end){
    int now = uptime();
    if(now != last){
      cnt++;
      last = now;
    }
  }
  return cnt;
}

static void burn_forever(void){
  volatile unsigned long long x = 0;
  for(;;) x++;
}

int
main(int argc, char **argv)
{
  // 우선순위(숫자 클수록 높음) 및 구성
  int pr_hi     = 9;   // HIGH (무한 루프)
  int pr_mid    = 5;   // MID  (즉시 종료 N개)
  int pr_low    = 1;   // LOW-WAIT (대기 후 측정)
  int n_mid     = 6;   // MID 개수
  int wait_low  = 60;  // LOW-WAIT이 대기하는 시간(틱)
  int win_high  = 50;  // HIGH 측정 윈도우(틱)
  int win_low   = 50;  // LOW-WAIT 측정 윈도우(틱)

  // 인자: priority_scheduler_test [hi] [mid] [low] [n_mid] [wait_low] [win_high] [win_low]
  if(argc >= 2) pr_hi    = atoi(argv[1]);
  if(argc >= 3) pr_mid   = atoi(argv[2]);
  if(argc >= 4) pr_low   = atoi(argv[3]);
  if(argc >= 5) n_mid    = atoi(argv[4]);
  if(argc >= 6) wait_low = atoi(argv[5]);
  if(argc >= 7) win_high = atoi(argv[6]);
  if(argc >= 8) win_low  = atoi(argv[7]);
  if(n_mid < 0) n_mid = 0;

  int pid_hi   = -1;
  int pid_loww = -1;

  // 1) HIGH: 무한 루프 (초기 1회 출력 + 측정 1회 출력 후 계속 burn)
  int rc = fork();
  if(rc < 0){ printf(1, "fork failed\n"); exit(); }
  if(rc == 0){
    int me = getpid();
    set_proc_priority(me, pr_hi);
    sleep(5); // 헤더와 겹침 방지 소량 지연
    printf(1, "HIGH  pid=%d pr=%d (infinite)\n", me, get_proc_priority(me));
    // 일정 윈도우 동안 실행 근사 측정 후 1회만 보고
    int re = measure_run_events(win_high);
    printf(1, "HIGH  pid=%d approx_run=%d over %d ticks\n", me, re, win_high);
    burn_forever(); // 이후 출력 없음 (콘솔 섞임 최소화)
    exit();
  }
  pid_hi = rc;

  // 2) LOW-WAIT: 낮게 시작 → 초기 스냅샷 → 충분히 대기(에이징) → 측정 후 종료
  rc = fork();
  if(rc < 0){ printf(1, "fork failed\n"); exit(); }
  if(rc == 0){
    int me = getpid();
    set_proc_priority(me, pr_low);
    sleep(8); // HIGH 다음에 깔끔히 찍히도록 약간 뒤로
    printf(1, "LOW   pid=%d init pr=%d (before wait)\n",
           me, get_proc_priority(me));
    sleep(wait_low);  // 여기서 에이징 기대
    int re = measure_run_events(win_low);
    printf(1, "LOW   pid=%d curr pr=%d approx_run=%d over %d ticks\n",
           me, get_proc_priority(me), re, win_low);
    exit();
  }
  pid_loww = rc;

  // 3) MID: 즉시 종료 N개 (출력 섞임 방지 위해 계단식 지연 후 한 줄만 출력)
  for(int i=0; i<n_mid; i++){
    rc = fork();
    if(rc < 0){ printf(1, "fork failed\n"); exit(); }
    if(rc == 0){
      int me = getpid();
      set_proc_priority(me, pr_mid);
      sleep(10 + i * 5); // i가 증가할수록 조금씩 늦게 찍음
      printf(1, "MID   pid=%d pr=%d (stopped)\n",
             me, get_proc_priority(me));
      exit();
    }
  }

  // 4) 부모: 주기적으로 HIGH/LOW-WAIT의 priority만 표로 출력(깔끔)
  printf(1, "\nTick\tHigh(pr)\tLowW(pr)\n");
  for(int t=0; t<=120; t++){
    if(t % 10 == 0){
      int hp = (pid_hi   > 0) ? get_proc_priority(pid_hi)   : -1;
      int lp = (pid_loww > 0) ? get_proc_priority(pid_loww) : -1;

      printf(1, "%d\t", t);
      print2(hp); printf(1, "\t\t");
      print2(lp); printf(1, "\n");
    }
    sleep(1);
  }

  // 5) 자식 수거: MID + LOW-WAIT
  int waits = 0;
  int expect = n_mid + 1; // (LOW-WAIT 1) + (MID n_mid)
  while(waits < expect){
    int done = wait();
    if(done > 0){
      printf(1, "Parent: Child pid=%d finished.\n", done);
      waits++;
    }else{
      break;
    }
  }

  // 6) HIGH 정리
  if(pid_hi > 0){
    kill(pid_hi);
    wait();
  }

  printf(1, "[priority_scheduler_test] done\n");
  exit();
}
