// main.c, 159
// This is kernel code for phase 9
//
// Team Name: Athwal (Members: Donald Murchison and Brian Souza)

#include "spede.h"      // given SPEDE stuff
#include "handlers.h"   // handler code
#include "tools.h"      // small functions for handlers
#include "proc.h"       // processes such as Init()
#include "types.h"      // data types
#include "events.h"     // events for kernel to serve
#include "services.h"
#include "FSdata.h"

// kernel's own data:
int current_pid;        // current selected PID; if 0, none selected
q_t ready_q, free_q, PF_q;    // processes ready to run and not used
pcb_t pcb[PROC_NUM];    // process control blocks
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks
int current_time;       //Current running time of OS
sem_t sem[Q_SIZE];      //Array of Semaphores
unsigned short *ch_p;   //check ps and a sem
port_t port[PORT_NUM];
mem_page_t mem_page[MEM_PAGE_NUM];
int kernel_MMU;        //address of main kernel address translation table

void Scheduler() {         // choose a PID as current_pid to load/run
//Phase 1
   if(current_pid!= 0){
     return; // if continue below, find one for current_pid
   }
   if(ready_q.size==0) {
      cons_printf("Kernel Panic: no process to run!\n"); // big problem!
      breakpoint();// to go into GDB;
   }

   current_pid = DeQ(&ready_q); //get next ready-to-run process as current_pid
   pcb[current_pid].state=RUN;  //update its state

//Phase 3
   ch_p[current_pid*80+42] = 0xf00 + 'R';

//Phase 1
   pcb[current_pid].cpu_time=0; //reset cpu_time count
}

// OS bootstrap from main() which is process 0, so we do not use this PID
int main() {
//Phase 1
   int i;
   struct i386_gate *IDT_p; // DRAM location where IDT is
   current_pid=0;

//Phase 2
   ch_p = (unsigned short *)0xb8000;
   current_time=0;
//Phase 8
    kernel_MMU = get_cr3();
    for(i=0;i<PROC_NUM;i++){
        pcb[i].MMU=0;
    }
//Phase 7
   for(i=0;i<MEM_PAGE_NUM;i++){
	mem_page[i].owner=0;
	mem_page[i].addr = (char *)(MEM_BASE + MEM_PAGE_SIZE*i);
   }

//Phase 6
   for (i=0; i<FD_NUM; i++){
       fd_array[i].owner=0;
   }
   root_dir[0].size = sizeof(root_dir);   // can only be assigned during runtime
   bin_dir[0].size = sizeof(bin_dir);     // even tho they're compiler-time sizes
   bin_dir[1].size = root_dir[0].size;    // otherwise, they would be recursive
   www_dir[0].size = sizeof(www_dir);     // definitions which compiler rejects
   www_dir[1].size = root_dir[0].size;

//Phase 5
  for(i=0;i<PORT_NUM;i++){
    port[i].owner=0;
  }
//Phase 1
   MyBzero((char*)&ready_q,sizeof(q_t));
   MyBzero((char*)&free_q,sizeof(q_t)); 
   MyBzero((char*)&sem,sizeof(sem_t)*20);  //Phase 3
   for(i=1;i<PROC_NUM;i++){
     EnQ(i,&free_q);
   }
   IDT_p = get_idt_base();  //init IDT_p (locate IDT location)
   cons_printf("IDT located at DRAM addr %x (%d).\n",IDT_p,IDT_p);
   
   fill_gate(&IDT_p[TIMER_EVENT],(int)TimerEvent,get_cs(),ACC_INTR_GATE,0);
   fill_gate(&IDT_p[SLEEP_EVENT],(int)SleepEvent,get_cs(),ACC_INTR_GATE,0);       //Phase 2
   fill_gate(&IDT_p[GETPID_EVENT],(int)GetPidEvent,get_cs(),ACC_INTR_GATE,0);     //Phase 2
   fill_gate(&IDT_p[SEMALLOC_EVENT],(int)SemAllocEvent,get_cs(),ACC_INTR_GATE,0); //Phase 3
   fill_gate(&IDT_p[SEMWAIT_EVENT],(int)SemWaitEvent,get_cs(),ACC_INTR_GATE,0);   //Phase 3
   fill_gate(&IDT_p[SEMPOST_EVENT],(int)SemPostEvent,get_cs(),ACC_INTR_GATE,0);   //Phase 3
   fill_gate(&IDT_p[SYSPRINT_EVENT],(int)SysPrintEvent,get_cs(),ACC_INTR_GATE,0); //Phase 4
   fill_gate(&IDT_p[PORT_EVENT],(int)PortEvent,get_cs(),ACC_INTR_GATE,0); //Phase 5
   fill_gate(&IDT_p[PORT_EVENT+1],(int)PortEvent,get_cs(),ACC_INTR_GATE,0); //Phase 5
   fill_gate(&IDT_p[PORTALLOC_EVENT],(int)PortAllocEvent,get_cs(),ACC_INTR_GATE,0); //Phase 5
   fill_gate(&IDT_p[PORTWRITE_EVENT],(int)PortWriteEvent,get_cs(),ACC_INTR_GATE,0); //Phase 5
   fill_gate(&IDT_p[PORTREAD_EVENT],(int)PortReadEvent,get_cs(),ACC_INTR_GATE,0); //Phase 5
   fill_gate(&IDT_p[FSFIND_EVENT],(int)FSfindEvent,get_cs(),ACC_INTR_GATE,0); //Phase 6
   fill_gate(&IDT_p[FSOPEN_EVENT],(int)FSopenEvent,get_cs(),ACC_INTR_GATE,0); //Phase 6
   fill_gate(&IDT_p[FSREAD_EVENT],(int)FSreadEvent,get_cs(),ACC_INTR_GATE,0); //Phase 6
   fill_gate(&IDT_p[FSCLOSE_EVENT],(int)FScloseEvent,get_cs(),ACC_INTR_GATE,0); //Phase 6
   
   fill_gate(&IDT_p[FORK_EVENT],(int)ForkEvent,get_cs(),ACC_INTR_GATE,0); //Phase 7
   fill_gate(&IDT_p[WAIT_EVENT],(int)WaitEvent,get_cs(),ACC_INTR_GATE,0); //Phase 7
   fill_gate(&IDT_p[EXIT_EVENT],(int)ExitEvent,get_cs(),ACC_INTR_GATE,0); //Phase 7
   
   fill_gate(&IDT_p[PF_EVENT],(int)PFEvent,get_cs(),ACC_INTR_GATE,0); //Phase 7
   outportb(0x21, ~(1+8+16));      //set PIC mask to open up for timer IRQ0 only
   NewProcHandler(Init);    //to create Init proc

   NewProcHandler(TermProc); 
   NewProcHandler(TermProc);
   Scheduler();             //to select current_pid (will be 1)

   Loader(pcb[current_pid].TF_p);// TF address of current_pid

   return 0; // compiler needs for syntax altho this statement is never exec
}

void Kernel(TF_t *TF_p) {   // kernel code exec (at least 100 times/second)
   pcb[current_pid].TF_p = TF_p;   //save TF_P into the PCB of current_pid
   switch(pcb[current_pid].TF_p->event_num){ 
    case TIMER_EVENT:     //Phase 1
         TimerHandler();
         break;
    case SLEEP_EVENT:     //Phase 2
         SleepHandler();
         break;
    case GETPID_EVENT:    //Phase 2
         GetPidHandler();
         break;
    case SEMALLOC_EVENT:  //Phase 3
         SemAllocHandler(TF_p->eax);
         break;
    case SEMWAIT_EVENT:   //Phase 3
         SemWaitHandler(TF_p->eax);
         break;
    case SEMPOST_EVENT:   //Phase 3
         SemPostHandler(TF_p->eax);
         break;
    case SYSPRINT_EVENT:  //Phase 4
         SysPrintHandler((char *)TF_p->eax);
         break;
    case PORT_EVENT:  //Phase 5
         PortHandler();
         break;
    case PORT_EVENT+1://Phase 5
         PortHandler();
         break;
    case PORTALLOC_EVENT:  //Phase 5
         PortAllocHandler(&TF_p->eax);
         break;
    case PORTWRITE_EVENT:  //Phase 5
         PortWriteHandler((char)TF_p->eax, TF_p->ebx);
         break;
    case PORTREAD_EVENT:  //Phase 5
         PortReadHandler((char *)TF_p->eax, TF_p->ebx);
         break;
    case FSFIND_EVENT:  //Phase 6
         FSfindHandler();
         break;
    case FSOPEN_EVENT:  //Phase 6
         FSopenHandler();
         break;
    case FSREAD_EVENT:  //Phase 6
         FSreadHandler();
         break;
    case FSCLOSE_EVENT:  //Phase 6
         FScloseHandler();
         break;
    case FORK_EVENT:  //Phase 7
      ForkHandler((char *)TF_p->eax, &TF_p->ebx);
      break;
    case WAIT_EVENT:  //Phase 7
      WaitHandler(&TF_p->eax);
      break;
    case EXIT_EVENT:
      ExitHandler(TF_p->eax);
      break;
    case PF_EVENT:
      PF_Handler();
      break;
    default:
         cons_printf("Kernel Panic: unknown event_num %d!\n",TF_p->event_num);
         breakpoint();
   }
   Scheduler();// to select current_pid (if needed)
   if(pcb[current_pid].MMU!=0){
       set_cr3(pcb[current_pid].MMU);
   }
   Loader(pcb[current_pid].TF_p); //with the TF address of current_pid
}

