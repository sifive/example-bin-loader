// Copyright 2020 SiFive, Inc
// See LICENSE for license details.
  .section .rodata
  .globl my_program
  .type my_program, @object
  .align 8
my_program:
  .incbin "myprogram.bin"

  .global my_program_size
  .type my_program_size, @object
  .align 8
my_program_size:
  .int my_program_size - my_program
