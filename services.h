#ifndef __SERVICES_H__
#define __SERVICES_H__

//Phase 2
void MySleep(int time);
int  GetPID(void);

//Phase 3
int SemAlloc(int passes);
void SemWait(int sid);
void SemPost(int sid);

//Phase 4
void SysPrint(char *str_to_print);

//Phase 5
int PortAlloc(void);
void PortWrite(char *p, int port_num);
void PortRead(char *p, int port_num);

#endif
