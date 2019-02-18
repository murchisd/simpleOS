// data.h, 159
// kernel data are all declared in main.c during bootstrap, but
// other kernel .c code must reference them as 'extern' (already declared)

#ifndef __DATA_H__                  // 'name-mangling' prevention
#define __DATA_H__                  // 'name-mangling' prevention

#include "types.h"                  // defines q_t, pcb_t, PROC_NUM, PROC_STACK_SIZE

//Phase 1
extern int current_pid;             // PID of current selected process to run, 0 means none
extern q_t ready_q, free_q;         // ready-to-run PID's, and un-used PID's
extern pcb_t pcb[PROC_NUM];         // 20 Process Control Blocks
extern char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // 20 process runtime stacks

//Phase 2
extern int current_time;            //Current running time of OS

//Phase 3
extern sem_t sem[Q_SIZE];            // Array of Sempahores, I think we only use one this lab
extern unsigned short *ch_p;         //Used to check proccess and a sem

//Phase 5
extern port_t port[PORT_NUM];

#endif // ifndef __DATA_H__         ('name-mangling' prevention)
