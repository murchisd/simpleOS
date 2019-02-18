# printer_msg.s, make printer print out a hello msg
# you must program this to make it work on both terminals
# add this to the bin/ directory of the file service

.text
.global _start
_start:
   mov $msg, %eax   # mov addr of 'msg' to a register (this is linker given, not our virtual addr)
   sub $_start, %eax   # subtract the starting addr of code from it, we get the distance between them
   add $0x40000000, %eax   # add 1G to the this distance, we get the correct virtual address
   push %eax   # save a copy of this calculated result onto stack
   int $105   # and call the SysPrint intr # to issue printer printing

   pop %eax   # pop from stack to eax as the exit #
   int $115   # and call the exit intr

.data
msg:
   .ascii "Hello, World! Team Athwal here...\n\r"     # declare a msg such as: Hello... (run demo to view how it should be)

