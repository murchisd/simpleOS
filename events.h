// events.h of events.S
// prototypes those coded in events.S

#ifndef __EVENTS_H__
#define __EVENTS_H__

// #include <spede/machine/pic.h> // shouldn't need this anymore

#define K_CODE 0x08         // kernel code segment # (register)
#define K_DATA 0x10         // kernel data segment # (register)
#define K_STACK_SIZE 16384  // kernel runtime stack byte size

#define TIMER_EVENT 32      // IDT entry #32, aka IRQ0, for timer device
#define SYSPRINT_EVENT 105  // 105 for now IRQ is for interrupt driven
			    // IDT entry #39, aka IRQ7, for print device
#define GETPID_EVENT 100    // IDT entry #100 for get pid kernel service
#define SLEEP_EVENT 101     // IDT entry #101 for sleep kernel service
#define SEMALLOC_EVENT 102  // IDT entry #102 for sem allocate service
#define SEMWAIT_EVENT 103   // IDT entry #103 for sem wait kernel service
#define SEMPOST_EVENT 104   // IDT entry #104 for sem post kernel service
#define PORT_EVENT 35
#define PORTALLOC_EVENT 106
#define PORTWRITE_EVENT 107
#define PORTREAD_EVENT 108
#define FSFIND_EVENT 109
#define FSOPEN_EVENT 110
#define FSREAD_EVENT 111
#define FSCLOSE_EVENT 112
#define FORK_EVENT 113
#define WAIT_EVENT 114
#define EXIT_EVENT 115
#define PF_EVENT 14

#ifndef ASSEMBLER  // skip below if ASSEMBLER defined (from an assembly code)
                   // since below is not in assembler syntax
__BEGIN_DECLS

#include "types.h"          // for 'TF_t' below

//Phase 1
void TimerEvent(void); 
void Loader(TF_t *); 

//Phase 2
void SleepEvent(void);
void GetPidEvent(void);

//Phase 3
void SemAllocEvent(void);
void SemWaitEvent(void);
void SemPostEvent(void);

//Phase 4
void SysPrintEvent(void);

//Phase 5
void PortEvent(void);
void PortAllocEvent(void);
void PortWriteEvent(void);
void PortReadEvent(void);

//Phase 6
void FSfindEvent(void);
void FSopenEvent(void);
void FSreadEvent(void);
void FScloseEvent(void);

//Phase 7
void ForkEvent(void);
void WaitEvent(void);
void ExitEvent(void);

//Phase 9
void PFEvent(void);
__END_DECLS

#endif // ifndef ASSEMBLER

#endif // ifndef __EVENTS_H__

