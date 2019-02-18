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

#ifndef ASSEMBLER  // skip below if ASSEMBLER defined (from an assembly code)
                   // since below is not in assembler syntax
__BEGIN_DECLS

#include "types.h"          // for 'TF_t' below

void TimerEvent(void);      // coded in events.S, assembler won't like this syntax
void SleepEvent(void);
void SysPrintEvent(void);
void GetPidEvent(void);
void SemAllocEvent(void);
void SemWaitEvent(void);
void SemPostEvent(void);
void Loader(TF_t *);        // coded in events.S

__END_DECLS

#endif // ifndef ASSEMBLER

#endif // ifndef __EVENTS_H__

