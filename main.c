// main.c, 159
// this is kernel code for phase 1
//
// Team Name: ??????? (Members: ?????? and ??????)

#include "spede.h"      // given SPEDE stuff
#include "handlers.h"   // handler code
#include "tools.h"      // small functions for handlers
#include "proc.h"       // processes such as Init()
#include "types.h"      // data types
#include "events.h"     // events for kernel to serve

// kernel's own data:
int current_pid;        // current selected PID; if 0, none selected
q_t ready_q, free_q;    // processes ready to run and not used
pcb_t pcb[PROC_NUM];    // process control blocks
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks

void Scheduler() {         // choose a PID as current_pid to load/run
   if(current_pid!= 0){
     return; // if continue below, find one for current_pid
   }
   if(ready_q.size==0) {
      cons_printf("Kernel Panic: no process to run!\n"); // big problem!
      breakpoint();// to go into GDB;
   }

   //get next ready-to-run process as current_pid
   current_pid = DeQ(&ready_q);
   //update its state
   pcb[current_pid].state=RUN;
   //reset cpu_time count
   pcb[current_pid].cpu_time=0;
}

// OS bootstrap from main() which is process 0, so we do not use this PID
int main() {
   int i;
   struct i386_gate *IDT_p; // DRAM location where IDT is

   //use tool function MyBzero to clear the two PID queues
   MyBzero(&ready_q,PROC_NUM);
   MyBzero(free_q.q,PROC_NUM);
   //queue free queue with PID 1~19
   for(i=1;i<PROC_NUM;i++){
     EnQ(i,&free_q);
   }
   free_q.size=PROC_NUM;

   //init IDT_p (locate IDT location)
   IDT_p = get_idt_base();
   cons_printf("IDT located at DRAM addr %x (%d).\n",&IDT_p,&IDT_p);
   //set IDT entry 32 like our timer lab
   fill_gate(&IDT_p[TIMER_EVENT],(int)TimerEvent,get_cs(),ACC_INTR_GATE,0);
   //set PIC mask to open up for timer IRQ0 only
   outportb(0x21, ~1);
   NewProcHandler(Init); //to create Init proc
   Scheduler(); //to select current_pid (will be 1)
   Loader(pcb[current_pid].TF_p);// TF address of current_pid

   return 0; // compiler needs for syntax altho this statement is never exec
}

void Kernel(TF_t *TF_p) {   // kernel code exec (at least 100 times/second)
   char key;
   //breakpoint();
   //save TF_P into the PCB of current_pid
   pcb[current_pid].TF_p = TF_p;
   // breakpoint();
   // switch according to the event_num in the TF TF_p points to {
   switch(pcb[current_pid].TF_p->event_num){ 
    //if it's timer event
    case TIMER_EVENT:
         //call timer event handler
         //breakpoint();
         TimerHandler();
         break;
      default:
         cons_printf("Kernel Panic: unknown event_num %d!\n",TF_p->event_num);
         breakpoint();
   }

   if(cons_kbhit()){//a key is pressed on Target PC {
      key = cons_getchar();
      //get the key

      //switch by the key obtained {
      switch(key){
         //if it's 'n'
         case 'n':
            //call NewProcHandler to create UserProc
            NewProcHandler(UserProc);
            break;
         //if it's 'b'
         case 'b':
            breakpoint();
     }
   }

   Scheduler();// to select current_pid (if needed)
   Loader(pcb[current_pid].TF_p); //with the TF address of current_pid
}

