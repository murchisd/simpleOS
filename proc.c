// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls

#include "spede.h"      // cons_xxx below needs
#include "data.h"       // current_pid needed below
#include "proc.h"       // prototypes of processes
#include "services.h"
#include "tools.h"

// Init PID 1, always ready to run, never preempted
void Init(void) {
   int i;
   char key;

//Phase 1
   while(1) {
       for(i=0;i<LOOP;i++) { // to cause approx 1 second of delay
           asm("inb $0x80"); //which delay .6 microsecond
       }

//Phase 4
       if(cons_kbhit()){       //a key is pressed on Target PC {
           key = cons_getchar(); //get the key

           switch(key){            //switch by the key obtained 
               case 'p':
                   SysPrint(" Hello, World! Team Athwal: Donald Murchison and Brian Souza\n\r");
                   break;
               case 'b':
                   breakpoint();
     	   }
       }
   }
}

//Phase 1
void UserProc(void) {   // PID 2, 3, 4, etc. mimicking a usual user process
   while(1) {
       cons_printf("%d..", GetPID());// (will change to GetPID call later)
       MySleep(GetPID());
     }
}

//Phase 6
void TermProc(void) {
   int i, len, my_port, exit_num;
   int pass_match;
   char login_str[BUFF_SIZE], passwd_str[BUFF_SIZE],
   cmd_str[BUFF_SIZE], cwd[BUFF_SIZE], str[BUFF_SIZE];

   //first, call PortAlloc() to allocate my_port
   my_port = PortAlloc();

   while(1){
       while(1){
           //prompt and get login and password strings
           PortWrite(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\r",my_port);
           PortWrite("Username:",my_port);
           PortRead(login_str,my_port);
           PortWrite("Password:",my_port);
           PortRead(passwd_str,my_port);
           len = MyStrlen(login_str);
           if(len==0) continue;
           if(len!=MyStrlen(passwd_str)) continue;

           pass_match = 1;
           for (i=0; i<len; i++) {
               if (login_str[i] != passwd_str[len-1-i]) {
                   pass_match = 0;
                   break;
               }
           }
           if (!pass_match) continue;
           PortWrite("      Welcome\n\r",my_port);
           PortWrite("      Services are pwd, cd dir-name, ls, cat filename, exit\n\r",my_port);
           MyStrcpy(cwd,"/");
           break;	
       }
       while(1){
           //prompt and enter command string
           PortWrite("Team Athwal:> ",my_port);
           PortRead(cmd_str,my_port);
           len = MyStrlen(cmd_str);
           if (len == 0) continue;
           //if command string is "exit\0" is -> break this loop
           if (MyStrcmp(cmd_str,"exit\0", 5)) break;
           if (MyStrcmp(cmd_str,"pwd\0",len)) {
               PortWrite(cwd,my_port);
               PortWrite("\n\r",my_port);
           }
           else if (MyStrcmp(cmd_str,"cd ",3)) {
               TermCd(cmd_str+3, cwd, my_port);
       	   }
           else if (MyStrcmp(cmd_str,"ls\0",len)) {
               TermLs(cwd, my_port);
           }
           else if (MyStrcmp(cmd_str,"cat ",4)) {
               TermCat(cmd_str+4, cwd, my_port);
           }
           else if(MyStrcmp(cmd_str,"echo\0",5)){
		sprintf(str,"%d (0x%x)\n\r",exit_num,exit_num);
		PortWrite(str,my_port);
	   }
           else {
		exit_num = TermBin(cmd_str,cwd,my_port);
           }
       }
   }
}

int TermBin(char *name, char *cwd, int my_port){
   char attr_data[BUFF_SIZE], str[BUFF_SIZE];
   attr_t *attr_p;
    int child_pid;
   
    FSfind(name, cwd, attr_data);
   
    if (MyStrlen(attr_data) == 0) {
       PortWrite("Not Found\n\r",my_port);
       return 0;
    }
    attr_p = (attr_t *) attr_data;  
    if (!QBIT_ON(attr_p->mode, A_XOTH)) { // mode is executable
       PortWrite("Not Executable\n\r",my_port);
       return 0;
    }
    child_pid = Fork(attr_p->data);
    sprintf(str,"%d (0x%x)\n\r",child_pid,child_pid);
    PortWrite(str,my_port);
    return Wait();
}    

void TermCd(char *name, char *cwd, int my_port) {
   char attr_data[BUFF_SIZE];
   attr_t *attr_p;

   if (MyStrlen(name) == 0) return;
   if (MyStrcmp(name,"\0", MyStrlen(name))) return;
   if (MyStrcmp(name,"/\0", MyStrlen(name)) || MyStrcmp(name,"..\0", MyStrlen(name))) {
       MyStrcpy(cwd,"/");
       return;
   }

   //call FSfind(), given name, cwd, and attr_data (to be filled)
   FSfind(name, cwd, attr_data);

   if (MyStrlen(attr_data) == 0) {
       PortWrite("Not Found\n\r",my_port);
       return;
   }
   attr_p = (attr_t *) attr_data;  
   //if attr_p->mode is a file (not directory),
   if (!A_ISDIR(attr_p->mode)) {
       PortWrite("cannot cd a file\n\r",my_port);
       return;
   }
   MyStrcat(cwd,name);
   MyStrcat(cwd,"/\0");
} 

// TermCat gets file services to prompt file (cwd/name) to terminal
void TermCat(char *name, char *cwd, int my_port) {
   char read_data[BUFF_SIZE], attr_data[BUFF_SIZE];
   attr_t *attr_p;
   int my_fd;

   //call FSfind(), given name, cwd, attr_data
   FSfind(name, cwd, attr_data);
   if (MyStrlen(attr_data) == 0) {
       PortWrite("Not Found\n\r",my_port);
		   return;
   }
   attr_p = (attr_t *) attr_data;  
   //if attr_p->mode is a directory (not file),
   if (A_ISDIR(attr_p->mode)) {
       PortWrite("cannot cat a directory\n\r",my_port);
       return;
   }
   //call FSopen(), given name and cwd to get my_fd returned
   my_fd = FSopen(name, cwd);
   while (1) {
       FSread(my_fd, read_data);
	   if (MyStrlen(read_data) == 0) break;
	   PortWrite(read_data,my_port);
	}
    //call FSclose() with my_fd to close/return it
	FSclose(my_fd);
}

// TermLs gets file services to prompt dir content to terminal
void TermLs(char *cwd, int my_port) {
   char ls_str[BUFF_SIZE], attr_data[BUFF_SIZE];
   attr_t *attr_p;
   int my_fd;

   //call FSfind(), given "", cwd, and attr_data (to be filled)
   FSfind("", cwd, attr_data);
   if (MyStrlen(attr_data) == 0) {
       PortWrite("Not Found\n\r",my_port);
       return;
   }
   attr_p = (attr_t *) attr_data;  

   //if attr_p->mode is a file (not directory),
   if (!A_ISDIR(attr_p->mode)) {
       PortWrite("cannot ls a file\n\r",my_port);
       return;
   }
   //  call FSopen(), given "" and cwd to get my_fd returned
   my_fd = FSopen("", cwd);
   while (1) {
       FSread(my_fd, attr_data);
       if (MyStrlen(attr_data) == 0) break;
       attr_p = (attr_t *) attr_data; 
       Attr2Str(attr_p, ls_str);
       PortWrite(ls_str, my_port);
   }

   //  call FSclose() with my_fd to close/return it
   FSclose(my_fd);
}

// make str from the attributes attr_p points
// str contains: attr_t and name (with p+1 to point to name)
void Attr2Str(attr_t *attr_p, char *str) {
   char *name = (char *)(attr_p + 1);
   sprintf(str, "     - - - -    SIZE %4d    NAME  %s\n\r", attr_p->size, name);
   if ( A_ISDIR(attr_p->mode) ) str[5] = 'D';          // mode is directory
   if ( QBIT_ON(attr_p->mode, A_ROTH) ) str[7] = 'R';  // mode is readable
   if ( QBIT_ON(attr_p->mode, A_WOTH) ) str[9] = 'W';  // mode is writable
   if ( QBIT_ON(attr_p->mode, A_XOTH) ) str[11] = 'X'; // mode is executable
}
