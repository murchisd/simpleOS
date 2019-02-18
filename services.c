//kernel services calls
//

#include "services.h"

void MySleep(int time){
  asm("pushl %%eax;
      movl %0, %%eax;
      int $101;
      popl %%eax"
      :
      : "g"(time)
      );
}

int GetPID(void){
  int pid;
  asm("pushl %%eax;
      int $100;
      movl %%eax, %0;
      popl %%eax"
      : "=g" (pid)
      :
      );
  return pid;
}

int SemAlloc(int passes){
  int sid;
  asm("pushl %%eax; 
       pushl %%ebx;
       movl %1, %%eax;
       int $102;
       movl %%ebx, %0;
       popl %%ebx;
       popl %%eax;"
       : "=g" (sid)
       : "g" (passes)
       );
  return sid;
}

void SemWait(int sid){
  asm("pushl %%eax;
       movl %0, %%eax;
       int $103;
       popl %%eax;"
       : 
       : "g" (sid)
       );
}

void SemPost(int sid){
  asm("pushl %%eax;
       movl %0, %%eax;
       int $104;
       popl %%eax;"
       :
       : "g" (sid)
       );
}

void SysPrint(char *str_to_print){
  asm("pushl %%eax;
       movl %0, %%eax;
       int $105;
       popl %%eax;"
       :
       : "g" ((int)str_to_print)
       );
}
