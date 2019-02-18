// handlers.h, 159

#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include "types.h"   // need definition of 'func_ptr_t' below

//Phase 1
void NewProcHandler(func_ptr_t);
void TimerHandler(void);

//Phase 2
void SleepHandler(void);
void GetPidHandler(void);

//Phase 3
void SemAllocHandler(int passes);
void SemWaitHandler(int sid);
void SemPostHandler(int sid);

//Phase 4
void SysPrintHandler(char *str_to_print);

//Phase 5
void PortWriteOne(int port_num);
void PortReadOne(int port_num);
void PortHandler(void);
void PortAllocHandler(int *eax);
void PortWriteHandler(char one, int port_num);
void PortReadHandler(char *one, int port_num);

//Phase 6
void FSfindHandler(void);
void FSopenHandler(void);
void FSreadHandler(void);
int FScanAccessFD(int fd, int owner);
int FSallocFD(int owner);
dir_t *FSfindName( char *name );
dir_t *FSfindNameSub( char *name, dir_t *this_dir );
void FSdir2attr( dir_t *dir_p, attr_t *attr_p );
void FScloseHandler(void);

//Phase 7
void ForkHandler(char *bin_code, int *child_pid);
void WaitHandler(int *exit_num_p);
void ExitHandler(int exit_num);
#endif
