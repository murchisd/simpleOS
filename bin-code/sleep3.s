# sleep3.s
# demo code, it calls Sleep(3)
# Enter "./build.pl sleep3.s" to compile and convert to text sleep3.txt
# build.pl does:
#    as --32 sleep.s -o sleep.o 
#    link386 -M -b 16m -nostartfiles sleep3.o -o sleep3
# sleep3.txt is to be included into file services

# linkable addresses shown in mapfile, "as -a sleep3.s" shows compile time addresses

.text               # code segment 
.global _start      # _start is main()

_start:             # instructions begin
   pushl $3         # playing with stack space
   popl %ebx
   movl %ebx, %eax
   int  $101        # call Sleep(3)

   movl $_start, %eax  # use code address as exit number
   int  $115        # call Exit(...)

.data               # examples of data declarations
x:
   .int 99          # integer, initialized 99
msg:
   .ascii "Hello, World!\n\r"  # compiler will null-terminate

