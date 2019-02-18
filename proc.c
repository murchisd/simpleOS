// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls

#include "spede.h"      // cons_xxx below needs
#include "data.h"       // current_pid needed below
#include "proc.h"       // prototypes of processes
#include "services.h"

// Init PID 1, always ready to run, never preempted
void Init(void) {
   int i;
   char key;

//Phase 1
   while(1) {
      for(i=0;i<LOOP;i++) { // to cause approx 1 second of delay
         asm("inb $0x80"); //which delay .6 microsecond
      }

//Phase 4
      if(cons_kbhit()){       //a key is pressed on Target PC {
        key = cons_getchar(); //get the key

        switch(key){            //switch by the key obtained 
          case 'p':
            SysPrint(" Hello, World! Team Athwal: Donald Murchison and Brian Souza\n\r");
            break;
          case 'b':
            breakpoint();
     	}
     }
   }
}

//Phase 1
void UserProc(void) {   // PID 2, 3, 4, etc. mimicking a usual user process
   while(1) {
     cons_printf("%d..", GetPID());// (will change to GetPID call later)
     MySleep(GetPID());
   }
}

//Phase 5
void TermProc(void) {
   int my_port;
   char str_read[BUFF_SIZE]; // size 101
   my_port = PortAlloc(); // init port device and port_t data associated
   while(1) {
      PortWrite("Hello, World! Team Athwal here!\n\r", my_port); // \r also!
      PortWrite("Now enter: ", my_port);
      PortRead(str_read, my_port);
      cons_printf("Read from port #%d: %s\n", my_port, str_read);
   }
}
