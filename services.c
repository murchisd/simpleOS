//kernel services calls
//

#include "services.h"

void MySleep(int time){
  asm("movl %0, %%eax;
      int $101;"
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
