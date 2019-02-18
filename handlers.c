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

//Phase 1
   int pid;

   if(free_q.size==0) { // this may occur for testing
      cons_printf("Kernel Panic: no more PID left!\n");
      breakpoint();                   // alternative: breakpoint() into GDB
   }
   
   pid = DeQ(&free_q);                       //get 'pid' from free_q
   MyBzero((char*)&pcb[pid],sizeof(pcb_t));  //use MyBzero tool to clear the PCB
   MyBzero((char*)&proc_stack[pid], PROC_STACK_SIZE);   //also, clear its runtime stack
   pcb[pid].state=READY;                     //update process state
   EnQ(pid,&ready_q);                        //queue 'pid' to be ready-to-run
   
   //point TF_p to highest area in stack (but has a space for a TF)
   pcb[pid].TF_p = (TF_t*) &proc_stack[pid][PROC_STACK_SIZE - sizeof(TF_t)];
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

//Phase 3
   if(pid>9){
      ch_p[pid*80+39] = 0xf00 + 0x30 + (pid/10);
   }
   ch_p[pid*80+40] = 0xf00 + 0x30 + (pid%10);     // show Pid
   ch_p[pid*80+42] = 0xf00 + 'r';
}

// count cpu_time of running process and preempt it if reaching limit
void TimerHandler(void) {
   int i;

//Phase 1
   pcb[current_pid].cpu_time++;   //upcount cpu_time of the process
   if(pcb[current_pid].cpu_time==TIME_LIMIT){
      pcb[current_pid].state = READY;         //update/downgrade its state
      ch_p[current_pid*80+42] = 0xf00 + 'r';  //Phase 3
      EnQ(current_pid,&ready_q);              //queue its PID back to ready-to-run PID queue
      current_pid=0;                          //reset current_pid (to 0)  // no running PID anymore
   }
  
//Phase 2
   current_time++;
   for(i=0;i<PROC_NUM;i++){
     if(pcb[i].state == SLEEP && pcb[i].wake_time==current_time){
        EnQ(i,&ready_q);
        pcb[i].state=READY;
        ch_p[i*80+42] = 0xf00 + 'r';    //Phase 3
     }
   }

//Phase 1
   outportb(0x20, 0x60);    //Don't forget: notify PIC event-handling done 
}

//Phase 2
void SleepHandler(void){
  int sleep = pcb[current_pid].TF_p->eax;
  pcb[current_pid].wake_time = current_time + 100 * sleep;
  pcb[current_pid].state = SLEEP;
  ch_p[current_pid*80+42] = 0xf00 + 'S'; //Phase 3
  current_pid=0;
}

void GetPidHandler(void){
  pcb[current_pid].TF_p->eax = current_pid;
}

//Phase 3
void SemAllocHandler(int passes){
  int i;
  for(i=0; i<Q_SIZE; i++){
    if(sem[i].owner==0){
      sem[i].passes=passes;
ch_p[50+i] = 0xf00 + 0x30+passes;
      MyBzero((char *)&(sem[i].wait_q),sizeof(q_t));
      sem[i].owner= current_pid;
      pcb[current_pid].TF_p->ebx = i;
      return;
    }
  }
  cons_printf("Kernel Panic: no more SID left!\n");
  breakpoint();                   // alternative: breakpoint() into GDB
  return;
}

void SemWaitHandler(int sid){
  if(sem[sid].passes>0){
    sem[sid].passes--;
    ch_p[50+sid] = 0xf00 + 0x30+sem[sid].passes;
    return;
  }
  EnQ(current_pid,&(sem[sid].wait_q));
  pcb[current_pid].state=WAIT;
  ch_p[current_pid*80+42] = 0xf00 + 'W';
  current_pid=0;
}

void SemPostHandler(int sid){
  int pid = DeQ(&(sem[sid].wait_q));
  if(pid==0){
    sem[sid].passes++;
    ch_p[50+sid] = 0xf00 + 0x30+sem[sid].passes;
    return;
  }
  EnQ(pid, &ready_q);
  pcb[pid].state=READY;
  ch_p[pid*80+42] = 0xf00 + 'r';
}

//Phase 4
void SysPrintHandler(char *p){
   int i, code;

   const int printer_port = 0x378;                // I/O mapped # 0x378
   const int printer_data = printer_port + 0;     // data register
   const int printer_status = printer_port + 1;   // status register
   const int printer_control = printer_port + 2;  // control register

// initialize printer port (check printer power, cable, and paper)
   outportb(printer_control, 16);             // 1<<4 is PC_SLCTIN
   code = inportb(printer_status);            // read printer status
   for(i=0; i<50; i++) asm("inb $0x80");      // needs some delay
   outportb(printer_control, 4 | 8 );         // 1<<2 is PC_INIT, 1<<3 PC_SLCTIN

   while(*p) {
      outportb(printer_data, *p);             // write char to printer data
      code = inportb(printer_control);        // read printer control
      outportb(printer_control, code | 1);    // 1<<0 is PC_STROBE
      for(i=0; i<50; i++) asm("inb $0x80");   // needs some delay
      outportb(printer_control, code);        // send original (cancel strobe)

      for(i = 0; i < LOOP*3; i++) {           // 3 seconds at most
         code = inportb(printer_status) & 64; // 1<<6 is PS_ACK
         if(code == 0) break;                 // printer ACK'ed
         asm("inb $0x80");                    // otherwise, wait 0.6 us, and loop
      }

      if(i == LOOP*3) {                        // if 3 sec did pass (didn't ACK)
         cons_printf(">>> Printer timed out!\n");
         break;   // abort printing
      }

      p++;        // move to print next character
   }
}

//Phase 5
void PortWriteOne(int port_num) {  // snd one char to port
   char one;
   if (port[port_num].write_q.size == 0 && port[port_num].loopback_q.size == 0) {
       port[port_num].write_ok = 1;
       return;
   }

   if (port[port_num].loopback_q.size != 0) {
       one = DeQ(&port[port_num].loopback_q);
   } else {
       one = DeQ(&port[port_num].write_q);
       SemPostHandler(port[port_num].write_sid);
   }
   outportb(port[port_num].IO+DATA,one); 
      //set write_ok of port data to 0     // will use write event below
  port[port_num].write_ok=0;
} // end of PortWriteOne(...

void PortReadOne(int port_num) {                // got one char from port
      char one;
      //call inportb to get 'one' from port DATA register
      one = inportb(port[port_num].IO+DATA);
      if(port[port_num].read_q.size==Q_SIZE){
         cons_printf("Kernel Panic: your typing on terminal is super fast!\n");
         return;
      }
      EnQ(one, &(port[port_num].read_q));
      EnQ(one, &(port[port_num].loopback_q));
      if(one=='\r'){
        EnQ('\n', &(port[port_num].loopback_q));
      }// add NL after CR

      SemPostHandler(port[port_num].read_sid);    // flow-control post/continue to read
   } // end of PortReadOne(...

void PortHandler(void) {         // IRQ3/4 event handler
      int port_num, intr_type;

       for(port_num=0;port_num<PORT_NUM-1;port_num++){
         intr_type = inportb(port[port_num].IO+IIR);
         if(intr_type==IIR_RXRDY){
           PortReadOne(port_num);
         }
         if(intr_type==IIR_TXRDY){
           PortWriteOne(port_num);
         }
         if(port[port_num].write_ok==1){
           PortWriteOne(port_num);
         }
      }
      //call outportb to dismiss both IRQ 3 and 4
      outportb(0x20,0x63);
      outportb(0x20,0x64);
   } // end PortHandler...

// allocate a serial port, set hardware and associated data:
// transmit speed 9600 bauds, clear IER, accept TXRDY and RXRDY events
//    COM1~8_IOBASE: 0x3f8 0x2f8 0x3e8 0x2e8 0x2f0 0x3e0 0x2e0 0x260
//    IIR: Intr Indicator Reg
//    IER: Intr Enable Reg
//    ETXRDY: Enable Xmit Ready
//    ERXRDY: Enable Recv Ready
//    MSR: Modem Status Reg
//    MCR: Modem Control Reg
//    LSR: Line Status Reg
//    CFCR: Char Format Control Reg
//    LSR_TSRE: Line Status Reg, Xmit+Shift Regs Empty

 void PortAllocHandler(int *eax) {
      int port_num, baud_rate, divisor;
      static int IO[PORT_NUM] = { 0x2f8, 0x3e8, 0x2e8 };

      //search each port data (loop index port_num from 0 to PORT_NUM-1):
      //   if its owner is zero, break loop // found one
      for(port_num=0;port_num<PORT_NUM-1;port_num++){
        if(port[port_num].owner==0){
          break;
        }
      }
      if(port_num==PORT_NUM){
         cons_printf("Kernel Panic: no port left!\n");
         return;
      }

      //write port_num at where eax point to // service call can return it
      *eax = port_num;

      //call MyBzero to clear the allocated port data
      //set its owner to current_pid
      //set its I/O to the I/O map # from the IO array (named above)
      //set its write_ok to 1                // OK to 1st write
      MyBzero((char *)&(port[port_num]),sizeof(port_t));
      port[port_num].owner=current_pid;
      port[port_num].IO = IO[port_num];
      port[port_num].write_ok=1;


// set baud, Control Format Control Register 7-E-1 (data- parity-stop bits)
// raise DTR, RTS of the serial port to start read/write
      baud_rate = 9600;
      divisor = 115200 / baud_rate;                        // time period of each baud
      //UPPERCASE are declared in headers
      outportb(port[port_num].IO+CFCR,CFCR_DLAB);
      outportb(port[port_num].IO+BAUDLO,LOBYTE(divisor));
      outportb(port[port_num].IO+BAUDHI,HIBYTE(divisor));
      outportb(port[port_num].IO+CFCR,CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
      outportb(port[port_num].IO+IER, 0);      
      outportb(port[port_num].IO+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
      asm("inb $0x80");      // needs some delay
      outportb(port[port_num].IO+IER, IER_ERXRDY|IER_ETXRDY);
   } // end PortAllocHandler...
void PortWriteHandler(char one, int port_num) { // to buffer one, actually
      if(port[port_num].write_q.size==Q_SIZE){ 
         cons_printf("Kernel Panic: terminal is not prompting (fast enough)?\n");
         breakpoint();
      }

      EnQ(one,&(port[port_num].write_q));            // buffer one
      if(port[port_num].write_ok==1){
      	PortWriteOne(port_num);       // can write one
      }
   }

void PortReadHandler(char *one, int port_num) { // to read from buffer, actually
      if(port[port_num].read_q.size==0){
         cons_printf("Kernel Panic: nothing in typing/read buffer?\n");
         return;
      }
      *one = DeQ(&(port[port_num].read_q));
} // end PortReadHandler

