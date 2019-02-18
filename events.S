// events.S, initial handling sequence of an event, build TF mainly

#include <spede/machine/asmacros.h> // do not use/include "spede.h"
#include "events.h"                 // for K_DATA, K_STACK_SIZE below

// set CPU DS and ES registers to K_DATA (data segment # registers)
#define SET_K_SEGS movw $(K_DATA), %ax; mov %ax, %ds; mov %ax, %es

.comm k_stack, K_STACK_SIZE, 1      // declare kernel stack space (global)
.text

//Phase 1
ENTRY(Loader)       // loads the trapframe of the selected process in order to run it
  movl 4(%esp), %eax       // copy whats above stack pointer
  movl %eax, %esp           // copy esp to eax
  popl %gs                    // pop regs
  popl %fs
  popl %es
  popl %ds                    
  popa                      // pop all
  addl $8, %esp              // skip timer event
  iret                      // pop eip, cs, eflags

// push event_num then go into kernel
ENTRY(TimerEvent)           // push eflags, cs, eip (by circuit)
   pushl $0
   pushl $TIMER_EVENT      
   jmp CommonPart

//Phase 2
ENTRY(SleepEvent)
   pushl $0
  pushl $SLEEP_EVENT
  jmp CommonPart

ENTRY(GetPidEvent)
   pushl $0
  pushl $GETPID_EVENT
  jmp CommonPart

CommonPart:
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

//Phase 3
ENTRY(SemAllocEvent)
   pushl $0
  pushl $SEMALLOC_EVENT
  jmp CommonPart

ENTRY(SemWaitEvent)
   pushl $0
  pushl $SEMWAIT_EVENT
  jmp CommonPart

ENTRY(SemPostEvent)
   pushl $0
  pushl $SEMPOST_EVENT
  jmp CommonPart

//Phase 4
ENTRY(SysPrintEvent)
   pushl $0
  pushl $SYSPRINT_EVENT
  jmp CommonPart

//Phase 5
ENTRY(PortEvent)
   pushl $0
  pushl $PORT_EVENT
  jmp CommonPart

ENTRY(PortAllocEvent)
   pushl $0
  pushl $PORTALLOC_EVENT
  jmp CommonPart

ENTRY(PortWriteEvent)
   pushl $0
  pushl $PORTWRITE_EVENT
  jmp CommonPart

ENTRY(PortReadEvent)
   pushl $0
  pushl $PORTREAD_EVENT
  jmp CommonPart

//Phase 6
ENTRY(FSfindEvent)
   pushl $0
  pushl $FSFIND_EVENT
  jmp CommonPart

ENTRY(FSopenEvent)
   pushl $0
  pushl $FSOPEN_EVENT
  jmp CommonPart

ENTRY(FSreadEvent)
   pushl $0
  pushl $FSREAD_EVENT
  jmp CommonPart

ENTRY(FScloseEvent)
   pushl $0
  pushl $FSCLOSE_EVENT
  jmp CommonPart

//Phase 7
ENTRY(ForkEvent)
   pushl $0
  pushl $FORK_EVENT
  jmp CommonPart

ENTRY(WaitEvent)
   pushl $0
  pushl $WAIT_EVENT
  jmp CommonPart

ENTRY(ExitEvent)
   pushl $0
  pushl $EXIT_EVENT
  jmp CommonPart

//Phase 9
ENTRY(PFEvent)
  pushl $PF_EVENT
  jmp CommonPart
