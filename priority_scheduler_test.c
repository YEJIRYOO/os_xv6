#include "types.h"
#include "stat.h"
#include "user.h"

int set_proc_priority(int pid, int prio);
int get_proc_priority(int pid);
int uptime(void); // xv6 system call

static void 
print_clean(int n){
  if(n < 10) printf(1, " %d", n);
  else printf(1, "%d", n);
}

// Approcimate execution event during given window
// Observe boudary change when th process is actually running
static int 
measure_run_events(int window_ticks){
  int end = uptime() + window_ticks;
  int last = uptime();
  int cnt = 0;

  // Monitor boundary through loop
  while(uptime() < end){
    int now = uptime();
    if(now != last){
      cnt++;
      last = now;
    }
  }
  return cnt;
}

// Infinite loop
static void 
burn_forever(void){
  volatile unsigned long long x = 0;
  for( ; ; ) x++;
}

int
main(int argc, char **argv)
{
  // Set priorities
  int priority_high=9;   // HIGH - infinite loop
  int priority_middle=5;   // MID - end immedietly
  int priority_low= 1;   // LOW-WAIT - wait and measure
  int mid_cnt= 6;   // MID count
  int wait_low=60;  // LOW-WAIT time
  int window_high=50;  // HIGH measurement window
  int window_low=50;  // LOW-WAIT measurement window

  if(argc>=2) priority_high=atoi(argv[1]);
  if(argc>=3) priority_middle=atoi(argv[2]);
  if(argc>=4) priority_low=atoi(argv[3]);
  if(argc>=5) mid_cnt=atoi(argv[4]);
  if(argc>=6) wait_low=atoi(argv[5]);
  if(argc>=7) window_high=atoi(argv[6]);
  if(argc>=8) window_low =atoi(argv[7]);
  if(mid_cnt<0) mid_cnt=0;

  int pid_high= -1;
  int pid_low= -1;

  // 1. HIGH: infinite loop (continue burn after initial print)
  int rc = fork();

    int p = getpid();
    set_proc_priority(p, priority_high);
    sleep(5);
    printf(1, "HIGH  pid=%d pr=%d (infinite)\n", p, get_proc_priority(p));

    int mesure = measure_run_events(window_high);
    printf(1, "HIGH  pid=%d approx_run=%d over %d ticks\n", p, mesure, window_high);
    burn_forever(); // burn loop
    exit();

  pid_high = rc;

  // 2. LOW-WAIT: srart low → capture early → wait for aging → measure
  rc = fork();

  int p = getpid();
  set_proc_priority(p,priority_low);
  sleep(8);
  printf(1, "LOW   pid=%d init pr=%d (before wait)\n",p, get_proc_priority(p));
  sleep(wait_low);  // wait for aging
  int mesure= measure_run_events(window_low);
  printf(1, "LOW   pid=%d curr pr=%d approx_run=%d over %d ticks\n",p, get_proc_priority(p), mesure, window_low);
  exit();
  pid_low=rc;

  // 3. MID: end immediatley 
  for(int i=0; i<mid_cnt; i++){
    rc= fork();
    int p= getpid();
    set_proc_priority(p, priority_middle);
    sleep(10+i*5); // i가 증가할수록 조금씩 늦게 찍음
    printf(1,"MID   pid=%d pr=%d (stopped)\n",p, get_proc_priority(p));
    exit();
  }

  // 4. Parent: print HIGH/LOW-WAIT priority in table form
  printf(1, "\nTick\tHigh(pr)\tLow (pr)\n");
  for(int t=0; t<=120; t++){
    if(t % 10 == 0){
      int hp= (pid_high > 0)?get_proc_priority(pid_high):-1;
      int lp= (pid_low > 0) ? get_proc_priority(pid_low):-1;

      printf(1,"%d\t", t);
      print_clean(hp); printf(1, "\t\t");
      print_clean(lp); printf(1, "\n");
    }
    sleep(1);
  }

  // 5. Clean child : MID + LOW-WAIT
  int waits= 0;
  int expect= mid_cnt+1; // (LOW-WAIT 1)+(MID n_mid)
  while(waits<expect){
    int done=wait();
    if(done>0){
      printf(1, "Parent: Child pid=%d finished.\n", done);
      waits++;
    }else{
      break;
    }
  }

  // 6. Clean HIGH
  if(pid_high > 0){
    kill(pid_high);
    wait();
  }

  printf(1, "priority_scheduler_test] done\n");
  exit();
}
