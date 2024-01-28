# Initial process execs /init.
# This code runs in user space.

#include "../syscall/syscall.h"
#include "../../arch/x86/traps.h"


# exec(init, argv)
.globl start
start:
  pushl $argv
  pushl $init
  pushl $0  // where caller pc would be
  movl $SYS_exec, %eax
  int $T_SYSCALL

# for(;;) exit();
exit:
  movl $SYS_exit, %eax
  int $T_SYSCALL
  jmp exit

init:
  .string "/bin/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0

