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

   while(1) {
      //cons_printf("1..");// (since Init has PID 1 as we know)
      for(i=0;i<LOOP;i++) { // to cause approx 1 second of delay
         asm("inb $0x80"); //which delay .6 microsecond
      }
   if(cons_kbhit()){//a key is pressed on Target PC {
      key = cons_getchar();
      //get the key

      //switch by the key obtained {
      switch(key){
         case 'p':
            SysPrint(" Hello, World! Team Athwal: Donald Murchison and Brian Souza\n\r");
            break;
         case 'b':
            breakpoint();
     	}
     }
   }
}

