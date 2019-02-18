#ifndef __SERVICES_H__
#define __SERVICES_H__

void MySleep(int time);
int  GetPID(void);
int SemAlloc(int passes);
void SemWait(int sid);
void SemPost(int sid);
void SysPrint(char *str_to_print);
#endif

