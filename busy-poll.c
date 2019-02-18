// busy-poll.c
// A busy-poll device driver example, to printer port

#include "spede.h"     // inportb(), outportb(), cons_printf() below
#define LOOP 1666667   // use this constant to alter loop number

//Phase 4
int main(void) {
   int i, code;
   char hello[] = "Hello, World!\n\r";   // newline (LF) carriage return (CR)
   char *p;

   const int printer_port = 0x378;                // I/O mapped # 0x378
   const int printer_data = printer_port + 0;     // data register
   const int printer_status = printer_port + 1;   // status register
   const int printer_control = printer_port + 2;  // control register

// initialize printer port (check printer power, cable, and paper)
   outportb(printer_control, 16);             // 1<<4 is PC_SLCTIN
   code = inportb(printer_status);            // read printer status
   for(i=0; i<50; i++) asm("inb $0x80");      // needs some delay
   outportb(printer_control, 4 | 8 );         // 1<<2 is PC_INIT, 1<<3 PC_SLCTIN

   p = hello;                                 // start from beginning of str
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
   }              // while(*p)

   return 0;
}

