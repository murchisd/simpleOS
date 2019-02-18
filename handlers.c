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
   MyBzero(&pcb[pid],sizeof(pcb_t));
   MyBzero(&proc_stack[pid], 4096);
   //use MyBzero tool to clear the PCB (indexed by 'pid')
   //also, clear its runtime stack
   pcb[pid].state=READY;
   //update process state
   EnQ(pid,&ready_q);
   //queue 'pid' to be ready-to-run

   //point TF_p to highest area in stack (but has a space for a TF)
   pcb[pid].TF_p = &proc_stack[pid][4032];
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
   //upcount cpu_time of the process (PID is current_pid)
   pcb[current_pid].cpu_time++;
   //cons_printf(" %d ", pcb[current_pid].cpu_time);
   if(pcb[current_pid].cpu_time==TIME_LIMIT){
      //update/downgrade its state
      pcb[current_pid].state = READY;
      //queue its PID back to ready-to-run PID queue
      EnQ(current_pid,&ready_q);
      //reset current_pid (to 0)  // no running PID anymore
      current_pid=0;
   }
   outportb(0x20, 0x60);
   //Don't forget: notify PIC event-handling done 
}

