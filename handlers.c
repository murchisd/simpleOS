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

   //Guessing because makes no sense to have MMU 0 when running
   //pcb[pid].MMU = kernel_MMU;

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
      for(port_num=0;port_num<PORT_NUM;port_num++){
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


// phase6 file services
void FSfindHandler(void) {
   char *name, *data;
   attr_t *attr_p;
   dir_t *dir_p;

   name = (char *)pcb[current_pid].TF_p->eax;
   data = (char *)pcb[current_pid].TF_p->ebx;

   dir_p = FSfindName(name);

   if(! dir_p) {      // dir_p == 0, not found
      data[0] = 0;    // null terminated, not found, return
      return;
   }

   attr_p = (attr_t *)data;
   FSdir2attr(dir_p, attr_p); // copy what dir_p points to to where attr_p points to

// should include filename (add 1 to length for null char)
   MyMemcpy((char *)(attr_p + 1), dir_p->name, MyStrlen(dir_p->name) + 1);
}

void FSopenHandler(void) {
   char *name;
   int fd;
   dir_t *dir_p;

   name = (char *)pcb[current_pid].TF_p->eax;

   fd = FSallocFD(current_pid);  // current_pid is owner of fd allocated

   if( fd == -1 ) {
      cons_printf("FSopenHandler: no more File Descriptor!\n");
      pcb[current_pid].TF_p->ebx = -1;
      return;
   }

   dir_p = FSfindName(name);
   if(! dir_p) {
      cons_printf("FSopenHandler: name not found!\n");
      pcb[current_pid].TF_p->ebx = -1;
      return;
   }

   fd_array[fd].item = dir_p;        // dir_p is the name
   pcb[current_pid].TF_p->ebx = fd;  // process gets this to future read
}

// Copy bytes from file into user's buffer. Returns actual count of bytes
// transferred. Read from fd_array[fd].offset (initially given 0) for
// buff_size in bytes, and record the offset. may reach EOF though...
void FSreadHandler(void) {
   int fd, result, remaining;
   char *read_data;
   dir_t *lp_dir;

   fd = pcb[current_pid].TF_p->eax;
   read_data = (char *)pcb[current_pid].TF_p->ebx;

   if(! FScanAccessFD(fd, current_pid)) {
      cons_printf("FSreadHandler: cannot read from FD!\n");
      read_data[0] = 0;  // null-terminate it
      return;
   }

   lp_dir = fd_array[fd].item;

   if( A_ISDIR(lp_dir->mode ) ) {  // it's a dir
// if reading directory, return attr_t structure followed by obj name.
// a chunk returned per read. `offset' is index into root_dir[] table.
      dir_t *this_dir = lp_dir;
      attr_t *attr_p = (attr_t *)read_data;
      dir_t *dir_p;

      if( BUFF_SIZE < sizeof( *attr_p ) + 2) {
         cons_printf("FSreadHandler: read buffer size too small!\n");
         read_data[0] = 0;  // null-terminate it
         return;
      }

// use current dir, advance to next dir for next time when called
      do {
         dir_p = ((dir_t *)this_dir->data);
         dir_p += fd_array[fd].offset ;

         if( dir_p->inode == END_INODE ) {
            read_data[0] = 0;  // EOF, null-terminate it
            return;
         }
         fd_array[fd].offset++;   // advance
      } while(dir_p->name == 0);

// MyBzero() fills buff with 0's, necessary to clean buff
// since FSdir2attr may not completely overwrite whole buff...
      MyBzero(read_data, BUFF_SIZE);
      FSdir2attr(dir_p, attr_p);

// copy obj name after attr_t, add 1 to length for null
      MyMemcpy((char *)( attr_p + 1 ), dir_p->name, MyStrlen( dir_p->name ) + 1);

   } else {  // a file, not dir
// compute max # of bytes can transfer then MyMemcpy()
      remaining = lp_dir->size - fd_array[fd].offset;

      if( remaining == 0 ) {
         read_data[0] = 0;  // EOF, null-terminate it
         return;
      }

      MyBzero(read_data, BUFF_SIZE);  // null termination for any part of file read

      result = remaining<100?remaining:100; // -1 saving is for last null

      MyMemcpy(read_data, &lp_dir->data[ fd_array[ fd ].offset ], result);

      fd_array[fd].offset += result;  // advance our "current" ptr
   }
}

// check ownership of fd and the fd is valid within range
int FScanAccessFD( int fd, int owner ) {
   if( fd_array[fd].owner == owner) return 1;
   return 0;     // not good
}

// Search our (fixed size) table of file descriptors. returns fd_array[] index
// if an unused entry is found, else -1 if all in use. if avail, then all
// fields are initialized.
int FSallocFD( int owner ) {
   int i;

   for(i=0; i<FD_NUM; i++) {
      if( 0 == fd_array[i].owner ) {
         fd_array[i].owner = owner;
         fd_array[i].offset = 0;
         fd_array[i].item = 0;     // NULL is (void *)0, spede/stdlib.h

         return i;
      }
   }

   return -1;   // no free file descriptors
}

dir_t *FSfindName( char *name ) {
   dir_t *starting;

// assume every path relative to root directory. Eventually, the user
// context will contain a "current working directory" and we can possibly
// start our search there
   if( name[0] == '/' ) {
      starting = root_dir;

      while( name[0] == '/' ) name++;

      if( name[0] == 0 ) return root_dir; // client asked for "/"
   } else {
// path is relative, so start off at CWD for this process
// but we don't have env var CWD, so just use root as well
      starting = root_dir; // should be what env var CWD is
   }

   if( name[0] == 0 ) return 0;

   return FSfindNameSub(name, starting);
}

// go searching through a single dir for a name match. use MyStrcmp()
// for case-insensitive compare. use '/' to separate directory components
// if more after '/' and we matched a dir, recurse down there
// RETURN: ptr to dir entry if found, else 0
// once any dir matched, don't return name which dir was matched
dir_t *FSfindNameSub( char *name, dir_t *this_dir ) {
   dir_t *dir_p = this_dir;
   int len = MyStrlen(name);
   char *p;

// if name is '.../...,' we decend into subdir
   if( ( p = strchr( name, '/' ) ) != 0) len = p - name;  // p = to where / is (high mem)

   for( ; dir_p->name; dir_p++ ) {
//      if((unsigned int)dir_p->name > 0xdfffff) return 0; // tmp bug-fix patch

      if( 1 == MyStrcmp( name, dir_p->name, len ) ) {
         if( p && p[1] != 0 ) { // not ending with name, it's "name/..."
// user is trying for a sub-dir. if there are more components, make sure this
// is a dir. if name ends with "/" we don't check. thus "hello.html/" is legal
            while( *p == '/' ) {
               p++;                           // skipping trailing /'s in name
               if( '\0' == *p ) return dir_p; // name "xxx/////" is actually legal
            }

// altho name given is "xxx/yyy," xxx is not a directory
            if(dir_p->mode != MODE_DIR) return 0; // bug-fix patch for "cat h/in"

            name = p;
            return FSfindNameSub(name, (dir_t *)dir_p->data);
         }
         return dir_p;
      }
   }

   return 0;   // no match found
}

// copy what dir_p points to (dir_t) to what attr_p points to (attr_t)
void FSdir2attr( dir_t *dir_p, attr_t *attr_p ) {
   attr_p->dev = current_pid;            // current_pid manages this i-node

   attr_p->inode = dir_p->inode;
   attr_p->mode = dir_p->mode;
   attr_p->nlink = ( A_ISDIR( attr_p->mode ) ) + 1;
   attr_p->size = dir_p->size;
   attr_p->data = dir_p->data;
}

void FScloseHandler(void) {
   int fd;

   fd = pcb[current_pid].TF_p->eax;

   if (FScanAccessFD(fd, current_pid))fd_array[fd].owner = 0;
   else  cons_printf("FScloseHandler: cannot close FD!\n");
}

void ForkHandler(char *bin_code, int *child_pid){
   int i, got, page_got[5];
   TF_t *TF_p;                                                 // local ptr, use below
   char *main_table, *code_table, *stack_table, *code_page, *stack_page; // easy naming
   got = 0;
   for( i=0; i <MEM_PAGE_NUM; i++) {
       if(mem_page[i].owner == 0) {
	       page_got[got++] = i;
		   if(got == 5) break;
	   }
   }
   if(got != 5) {
       cons_printf("Kernel Panic: no memory page available!\n");
       *child_pid = 0;
       return;
   }
   if(free_q.size == 0) {
       cons_printf("Kernel Panic: no PID available!\n");
       *child_pid = 0;  // no PID can be returned
       return;
   }
   main_table = (char *)mem_page[page_got[0]].addr;
   code_table = (char *)mem_page[page_got[1]].addr;
   stack_table = (char *)mem_page[page_got[2]].addr;
   code_page = (char *)mem_page[page_got[3]].addr;
   stack_page = (char *)mem_page[page_got[4]].addr;

   *child_pid = DeQ(&free_q);
   for( i=0; i <5; i++) {
       mem_page[page_got[i]].owner = *child_pid;
	     MyBzero((char *)mem_page[page_got[i]].addr, MEM_PAGE_SIZE);
   }
   MyBzero((char*)&pcb[*child_pid], sizeof(pcb_t));  
   EnQ(*child_pid, &ready_q);
   pcb[*child_pid].state = READY;
   pcb[*child_pid].ppid = current_pid;
   //set its TF_p to 2G (0x80000000) - size of trapframe type  <------- ***
   //set the MMU in the PCB of the new process to main_table <------- ***
   pcb[*child_pid].TF_p = (TF_t *)(0x80000000 - sizeof(TF_t));
   pcb[*child_pid].MMU = (int)main_table;
   MyMemcpy((char *)code_page, (char *)bin_code, MEM_PAGE_SIZE);
   TF_p = (TF_t *)(stack_page + MEM_PAGE_SIZE  - sizeof(TF_t)); 
   TF_p->eip = 0x40000000;
   TF_p->eflags = EF_DEFAULT_VALUE|EF_INTR;
   TF_p->cs = get_cs();
   TF_p->ds = get_ds();
   TF_p->es = get_es();
   TF_p->fs = get_fs();
   TF_p->gs = get_gs();

    MyMemcpy((char *)main_table, (char *)kernel_MMU, 16);
    MyMemcpy((char *)&main_table[4*256],(char *)&code_table, 4);
    main_table[4*256] = 0x3 | main_table[4*256];
    MyMemcpy((char *)&main_table[4*511],(char *)&stack_table,4);
    main_table[4*511]=0x3|main_table[4*511];
    MyMemcpy((char *)&code_table[0], (char *)&code_page,4);
    code_table[0]=0x3|code_table[0];
    MyMemcpy((char *)&stack_table[4*1023],(char *)&stack_page,4);
    stack_table[4*1023]=0x3|stack_table[4*1023];

}

void WaitHandler(int *exit_num_p){
   int child_pid, page_index, i, tmp;
   for(child_pid=0; child_pid<PROC_NUM; child_pid++) {
       if(pcb[child_pid].ppid == current_pid && pcb[child_pid].state == ZOMBIE) break;
   }
   if(child_pid == PROC_NUM) {
       pcb[current_pid].state = WAIT;
       current_pid = 0;
       return;
   }
   set_cr3(pcb[child_pid].MMU);
   *exit_num_p = pcb[child_pid].TF_p->eax;
   set_cr3(kernel_MMU);
   EnQ(child_pid, &free_q);
   pcb[child_pid].state = FREE;
   for( page_index=0; page_index <MEM_PAGE_NUM ; page_index++) {
       if(mem_page[page_index].owner == child_pid) mem_page[page_index].owner=0;
   }
   if(PF_q.size != 0) {
       for(i=0;i<5;i++){
	tmp = DeQ(&PF_q);
        if(tmp==0) break;
	EnQ(tmp,&ready_q);
	pcb[tmp].state=READY;
       }
   }
}

void ExitHandler(int exit_num){
   int ppid, *exit_num_p, page_index, i, tmp;
    
    ppid = pcb[current_pid].ppid;
    if(pcb[ppid].state!=WAIT){
        pcb[current_pid].state=ZOMBIE;
        current_pid=0;
        return;
    }

    pcb[ppid].state=READY;
    EnQ(ppid, &ready_q);
    exit_num_p = (int *)&pcb[ppid].TF_p->eax;
    *exit_num_p = exit_num;
    
    for(page_index=0; page_index <MEM_PAGE_NUM ; page_index++) {
       if(mem_page[page_index].owner == current_pid) mem_page[page_index].owner=0;
    }
    //DeQ 5 pages from PF_q
    //if some need more than one page they will be re Queued
    if(PF_q.size != 0) {
      for(i=0;i<5;i++){
	tmp = DeQ(&PF_q);
        if(tmp==0) break;
	EnQ(tmp,&ready_q);
	pcb[tmp].state=READY;
       }
    }	
    EnQ(current_pid, &free_q);
    pcb[current_pid].state = FREE;
    current_pid=0;
    set_cr3(kernel_MMU);

}

void PF_Handler(void) {
   int i;
   unsigned int fault_addr, main_addr, sub_addr;
   int * main_table_addr;
   int *sub_table, *runtime_page; 
   
   main_table_addr = (int *)pcb[current_pid].MMU;
   fault_addr = (unsigned int) get_cr2(); 
   main_addr = (fault_addr >> 22);
   sub_addr = (fault_addr >> 12)&0x3ff;
   
   //Set runtime_page to entry - clear flags so we can use as table if valid
   sub_table =(int *)((int)main_table_addr[main_addr]&0xfffff000);
   //Check if the entry had flags set before clearing
   //If not get a new mem page addr and overwrite value in runtime_page
   if(!(main_table_addr[main_addr] & 0x1)){ 
       for( i=0; i <MEM_PAGE_NUM; i++) {
          if(mem_page[i].owner == 0) {
            sub_table = (int *)mem_page[i].addr;
            //Set the new page as entry in maintable with flags
	          main_table_addr[main_addr]=(int)sub_table|0x3;
            mem_page[i].owner = current_pid;
	          MyBzero((char *)mem_page[i].addr, MEM_PAGE_SIZE);
	    break; 	
          }
       }

       if(i ==  MEM_PAGE_NUM) {
          EnQ(current_pid, &PF_q);
          pcb[current_pid].PF_addr = fault_addr;
          pcb[current_pid].state = WAIT;
          current_pid = 0;
          return;
      }
   }


   //Set sub_table to entry in runtimepage - clear flags so we can use as table if valid
   runtime_page =(int *)((int)sub_table[sub_addr]&0xfffff000);
   //Check if the entry had flags set before clearing
   //If not get a new mem page addr and overwrite value in runtime_page
   if(!(sub_table[sub_addr] & 0x1)){ 
       for( i=0; i <MEM_PAGE_NUM; i++) {
          if(mem_page[i].owner == 0) {
            runtime_page = (int *)mem_page[i].addr;
	          sub_table[sub_addr]=(int)runtime_page|0x3;
            mem_page[i].owner = current_pid;
	          MyBzero((char *)mem_page[i].addr, MEM_PAGE_SIZE);
	    break; 	
          }
       }

       if(i ==  MEM_PAGE_NUM) {
          EnQ(current_pid, &PF_q);
          pcb[current_pid].PF_addr = fault_addr;
          pcb[current_pid].state = WAIT;
          current_pid = 0;
          return;
      }
   }
}
