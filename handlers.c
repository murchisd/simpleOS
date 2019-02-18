// handlers.c, 159

#include "spede.h"
#include "types.h"
#include "handlers.h"
#include "tools.h"
#include "data.h"
#include "proc.h"

// to create process, alloc PID, PCB, and stack space
// build TF into stack, set PCB, register PID to ready_q
void NewProcHandler(func_ptr_t p) {  // arg: where process code starts
   int pid;

   if(free_q.size==0) { // this may occur for testing
      cons_printf("Kernel Panic: no more PID left!\n");
      breakpoint();                   // alternative: breakpoint() into GDB
   }
   
   pid = DeQ(&free_q);
   //get 'pid' from free_q
   MyBzero((char*)&pcb[pid],sizeof(pcb_t));
   MyBzero((char*)&proc_stack[pid], 4096);
   //use MyBzero tool to clear the PCB (indexed by 'pid')
   //also, clear its runtime stack
   pcb[pid].state=READY;
   if(pid>9){
      ch_p[pid*80+39] = 0xf00 + 0x30 + (pid/10);
   }
   ch_p[pid*80+40] = 0xf00 + 0x30 + (pid%10);     // show Pid
   ch_p[pid*80+42] = 0xf00 + 'r';
   //update process state
   EnQ(pid,&ready_q);
   //queue 'pid' to be ready-to-run

   //point TF_p to highest area in stack (but has a space for a TF)
   pcb[pid].TF_p = (TF_t*) &proc_stack[pid][4032];
   //then fill out the eip of the TF
   pcb[pid].TF_p->eip = (int)p;
   //the eflags in the TF becomes: EF_DEFAULT_VALUE|EF_INTR; // EFL will enable intr!
   pcb[pid].TF_p->eflags = EF_DEFAULT_VALUE|EF_INTR;
   //the cs in the TF is get_cs();   // duplicate from current CPU
   pcb[pid].TF_p->cs = get_cs();
   //the ds in the TF is get_ds();   // duplicate from current CPU
   pcb[pid].TF_p->ds = get_ds();
   //the es in the TF is get_es();   // duplicate from current CPU
   pcb[pid].TF_p->es = get_es();
   //the fs in the TF is get_fs();   // duplicate from current CPU
   pcb[pid].TF_p->fs = get_fs();
   //the gs in the TF is get_gs();   // duplicate from current CPU
   pcb[pid].TF_p->gs = get_gs();
   //breakpoint();
}

// count cpu_time of running process and preempt it if reaching limit
void TimerHandler(void) {
   int i;
   //upcount cpu_time of the process (PID is current_pid)
   pcb[current_pid].cpu_time++;
   current_time++;
   //cons_printf(" %d ", pcb[current_pid].cpu_time);
   if(pcb[current_pid].cpu_time==TIME_LIMIT){
      //update/downgrade its state
      pcb[current_pid].state = READY;
      if(current_pid>9){
          ch_p[current_pid*80+39] = 0xf00 + 0x30 + (current_pid/10);
      }
      ch_p[current_pid*80+40] = 0xf00 + 0x30 + (current_pid%10);     // show Pid
      ch_p[current_pid*80+42] = 0xf00 + 'r';
      //queue its PID back to ready-to-run PID queue
      EnQ(current_pid,&ready_q);
      //reset current_pid (to 0)  // no running PID anymore
      current_pid=0;
   }
   for(i=0;i<PROC_NUM;i++){
     if(pcb[i].state == SLEEP && pcb[i].wake_time==current_time){
        EnQ(i,&ready_q);
        pcb[i].state=READY;
        if(i>9){
          ch_p[i*80+39] = 0xf00 + 0x30 + (i/10);
        }
        ch_p[i*80+40] = 0xf00 + 0x30 + (i%10);     // show Pid
        ch_p[i*80+42] = 0xf00 + 'r';
     }
   }
   outportb(0x20, 0x60);
   //Don't forget: notify PIC event-handling done 
}

void SleepHandler(void){
  int sleep = pcb[current_pid].TF_p->eax;
  pcb[current_pid].wake_time = current_time + 100 * sleep;
  pcb[current_pid].state = SLEEP;
  if(current_pid>9){
      ch_p[current_pid*80+39] = 0xf00 + 0x30 + (current_pid/10);
  }
  ch_p[current_pid*80+40] = 0xf00 + 0x30 + (current_pid%10);     // show Pid
  ch_p[current_pid*80+42] = 0xf00 + 'S';
  current_pid=0;
}

void GetPidHandler(void){
  pcb[current_pid].TF_p->eax = current_pid;
}

void SemAllocHandler(int passes){
  int i;
  for(i=0; i<Q_SIZE; i++){
    if(sem[i].owner==0){
      sem[i].passes=passes;
      ch_p[i*80+10] = 0xf00 + 0x30+passes;
      MyBzero((char *)&(sem[i].wait_q),sizeof(q_t));
      sem[i].owner= current_pid;
      pcb[current_pid].TF_p->ebx = i;
      return;
    }
  }
  cons_printf("Kernel Panic: no more PID left!\n");
  breakpoint();                   // alternative: breakpoint() into GDB
  return;
}

void SemWaitHandler(int sid){
  if(sem[sid].passes>0){
    sem[sid].passes--;
    ch_p[sid*80+10] = 0xf00 + 0x30+sem[sid].passes;
  }
  else{
    EnQ(current_pid,&(sem[sid].wait_q));
    pcb[current_pid].state=WAIT;
    if(current_pid>9){
      ch_p[current_pid*80+39] = 0xf00 + 0x30 + (current_pid/10);
    }
    ch_p[current_pid*80+40] = 0xf00 + 0x30 + (current_pid%10);     // show Pid
    ch_p[current_pid*80+42] = 0xf00 + 'W';
    current_pid=0;
  }
}

void SemPostHandler(int sid){
  int pid = DeQ(&(sem[sid].wait_q));
  if(pid==0){
    sem[sid].passes++;
    ch_p[sid*80+10] = 0xf00 + 0x30+sem[sid].passes;
  }
  else{
    EnQ(pid, &ready_q);
    pcb[pid].state=READY;
    if(pid>9){
      ch_p[pid*80+39] = 0xf00 + 0x30 + (pid/10);
    }
    ch_p[pid*80+40] = 0xf00 + 0x30 + (pid%10);     // show Pid
    ch_p[pid*80+42] = 0xf00 + 'r';
  }
}

