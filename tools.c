// tools.c, 159

#include "spede.h"
#include "types.h"
#include "data.h"

// clear DRAM data blocks by filling zeroes
void MyBzero(char *p, int size) {
   int i=0;
   while(i<size){
     *p=0x00;
     p++;
     i++;
   }
}

// dequeue, return 1st integer in array, and move all forward
// if queue empty, return 0
int DeQ(q_t *p) { // return 0 if q[] is empty
   int i, data = 0;

   if(p->size==0){
     return data;
   }
   data = p->q[0];
   p->size--;
   for(i=1; i<=p->size; i++){
     p->q[i-1]=p->q[i];
   }
   //Shouldn't need to clear last element
   //Since size determines end of array
   return data;
}

// enqueue integer to next available slot in array, size is index
void EnQ(int data, q_t *p) {
   if(p->size==Q_SIZE) {
      cons_printf("Kernel Panic: queue is full, cannot EnQ!\n");
      breakpoint();       // alternative: breakpoint() into GDB
   }
   p->q[p->size]=data;
   p->size++;
}

