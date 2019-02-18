// proc.h, 159

#ifndef __PROC_H__
#define __PROC_H__

void Init(void);      // PID 1, eternal, never preempted
void UserProc(void);  // PID 2, 3, ...

#endif
