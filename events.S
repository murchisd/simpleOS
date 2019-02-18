// events.S, initial handling sequence of an event, build TF mainly

#include <spede/machine/asmacros.h> // do not use/include "spede.h"
#include "events.h"                 // for K_DATA, K_STACK_SIZE below

// set CPU DS and ES registers to K_DATA (data segment # registers)
#define SET_K_SEGS movw $(K_DATA), %ax; mov %ax, %ds; mov %ax, %es

.comm k_stack, K_STACK_SIZE, 1      // declare kernel stack space (global)
.text

// loads the trapframe of the selected process in order to run it
ENTRY(Loader)
  movl 4(%esp), %eax       // copy whats above stack pointer
  movl %eax, %esp           // copy esp to eax
  pop %gs                    // pop regs
  pop %fs
  pop %es
  pop %ds                    
  popa                      // pop all
  addl $4, %esp              // skip timer event
  iret                      // pop eip, cs, eflags

// push event_num then go into kernel
ENTRY(TimerEvent)           // push eflags, cs, eip (by circuit)
   pushl $TIMER_EVENT       // push var with $ 
   pusha                    // push all: e-a/c/d/b-x, e-s/b-p, e-s/d-i
   pushl %ds                 // push 4 data seg registers
   pushl %es
   pushl %fs
   pushl %gs
   movl %esp, %ecx          // copy TF addr (esp) to ecx
   cld                      // clear direction flag bit
   SET_K_SEGS               // set kernel data segment registers
   leal k_stack + K_STACK_SIZE, %esp  // point esp to kernel stack
   pushl %ecx                // push ecx (TF addr) there
   call CNAME(Kernel)       // call Kernel code
   // Unclear if we need to pop everything and ret here

