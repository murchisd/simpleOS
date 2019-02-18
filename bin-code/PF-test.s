# PF_test.s
#
# program code that
# use its own PID times 1M (0x100000), pluse 1G (0x40000000) as the
# address to write its PID to, # e.g., if PID is 4, write 4 to addr
# 1G+4M; if 5, write 5 to 1G+5M; etc.
# exit with the address of writing
...
