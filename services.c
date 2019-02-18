//kernel services calls

#include "data.h"
#include "tools.h"
#include "services.h"
//Phase 2
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

//Phase 3
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

//Phase 4
void SysPrint(char *str_to_print){
   asm("pushl %%eax;
       movl %0, %%eax;
       int $105;
       popl %%eax;"
       :
       : "g" ((int)str_to_print)
       );
}

//Phase 5
int PortAlloc(void) { // request a serial port # to read/write
   int port_num;
   asm("pushl %%eax;
       int $106;
       movl %%eax, %0;
       popl %%eax;"
       : "=g" (port_num)
       :
       );
   MySleep(1); 		//after getting port_num, Sleep for a second
   
   port[port_num].write_sid = SemAlloc(20);
   port[port_num].read_sid = SemAlloc(0);
   port[port_num].read_q.size=0;
   return port_num;
}

void PortWrite(char *p, int port_num) { // prompt char string to terminal
// loop thru string: sem-wait and then call "int xxx" to write a char
   while (*p) {//loop while what p points to has something to print (not null)
       SemWait(port[port_num].write_sid);      //call SemWait(?) for checking if the write buffer has space
       asm("pushl %%eax;
           pushl %%ebx;
           movl %0, %%eax; 
           movl %1, %%ebx;
           int $107;
           popl %%ebx;
           popl %%eax;"
           :
           : "g" (*p), "g" (port_num) // two input items
           );
       p++; //advance the address in p to next character
   }
}

void PortRead(char *p, int port_num) { // to read terminal KB
   int size;
   // loop: sem-wait then "int xxx" to get a char until \r (or size limit)
   size = 0; 
   while(1) {             // break at \r or at BUFF_SIZE-1
       SemWait(port[port_num].read_sid);      // flow-control
       asm("pushl %%eax;
           pushl %%ebx;
           movl %0, %%eax;
           movl %1, %%ebx;
           int $108;
           popl %%ebx;
           popl %%eax;"
           :
           : "g" (p), "g" (port_num) // two inputs: p (addr for getting a char) and port #
           );
       if(*p=='\r') break;	//break loop if what p points to is filled out with '\r'
       p++;			//advance addr p to next character
       size++;			//increment size by 1
       if(size == BUFF_SIZE-1) break;		//break loop if size reaching BUFF_SIZE-1
   } 
   *p = '\0';		//where p points to is set to '\0'  // null-terminate str, overwirte \r
}

void FSfind(char *name, char *cwd, char *data){ // find CWD/name, return attr data
   char tmp[BUFF_SIZE];
   MyStrcpy(tmp, cwd);
   MyStrcat(tmp, name);
   asm("pushl %%eax;
       pushl %%ebx;
       movl %0, %%eax;
       movl %1, %%ebx;
       int $109;
       popl %%ebx;
       popl %%eax;"
       :
       : "g" ((int)tmp), "g"((int)data)
       );
}

int FSopen(char *name, char *cwd) {              // alloc FD to open CWD/name
   int fd;
   char tmp[BUFF_SIZE];
   MyStrcpy(tmp, cwd);
   MyStrcat(tmp, name);
   asm("pushl %%eax;
       pushl %%ebx;
       movl %1, %%eax;
       int $110;
       movl %%ebx, %0;
       popl %%ebx;
       popl %%eax;"
       : "=g" (fd)
       : "g" (tmp)
       );
   return fd;
}

void FSread(int fd, char *data) {                // read FD into data buffer
   asm("pushl %%eax;
       pushl %%ebx;
       movl %0, %%eax;
       movl %1, %%ebx;
       int $111;
       popl %%ebx;
       popl %%eax;"
       :
       : "g" (fd), "g" (data)
       );
}

void FSclose(int fd){                           // close allocated fd (FD)
   asm("pushl %%eax;
       movl %0, %%eax;
       int $112;
       popl %%eax;"
       :
       : "g" (fd)
       );
}

int Fork(char *p){  // parent creates child, child PID returns
  int child_pid;
   asm("pushl %%eax;
       pushl %%ebx;
       movl %1, %%eax;
       int $113;
       movl %0, %%ebx;
       popl %%ebx;
       popl %%eax;"
       : "=g" (child_pid)
       : "g" ((int)p) 
       );
  return child_pid;
}

int Wait(void){ // parent process waits exit_num from exiting child
  int exit_num_p;
  asm("pushl %%eax;
      int $114;
      movl %0, %%eax;
      popl %%eax;"
      : "=g" (exit_num_p)
      :
      );
  return exit_num_p;
}

void Exit(int exit_num){
  asm("pushl %%eax;
      movl %0, %%eax;
      int $115;
      popl %%eax;"
      :
      : "g" (exit_num)
      );
}
