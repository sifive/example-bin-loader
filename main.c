/* Copyright 2020 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <metal/cpu.h>
#include <metal/lock.h>

#define METAL_CPU_GET_CSR(reg, value)                                          \
    __asm__ volatile("csrr %0, " #reg : "=r"(value));

#define METAL_CPU_SET_CSR(reg, value)                                          \
    __asm__ volatile("csrw " #reg ", %0" : : "r"(value));

#define PAYLOAD_DEST	0x60000000UL

/*
 * Pick up the program binary and its size defined in program.S
 */
uintptr_t __attribute__((weak)) my_program = 0x1949;
int __attribute__((weak)) my_program_size = 0;

/*
 * Have a lock to protect harts_checkin count
 */
METAL_LOCK_DECLARE(my_lock);

/*
 * This is a count of the number of harts who have executed their main function.
 * Writes to this variable must be protected by my_lock.
 */
volatile int harts_checkin = 0;

/*
 * Libc constructors run during _start while secondary harts are waiting for the boot hart
 * to finish initialization, so we can be sure that the lock is initialized before secondary
 * harts try to take it
 */
__attribute__((constructor))
void init_lock_and_payload(void) {
    int rc = metal_lock_init(&my_lock);
    if(rc != 0) {
        puts("Failed to initialize my_lock\n");
        exit(1);
    }

    /* Ensure that the lock is initialized before any readers of the lock */
    __asm__ ("fence rw,w"); /* Release semantics */

    /* Copy the program binary from rodata to its destination */
    memcpy((void*)PAYLOAD_DEST, (void*)&my_program, my_program_size);
}

/*
 * We redefine secondary_main() to allow all harts to run the loaded execuatable
 * Or hartid != 0 to execute other_main() to run the loaded execuatable
 */
int secondary_main(void) {
   metal_lock_take(&my_lock);

   harts_checkin += 1;

   metal_lock_give(&my_lock);

   int num_harts = metal_cpu_get_num_harts();

   printf("LOADING DONE! \n");

   /* One can choose to add to a check here to decide which harts to trap! */
   METAL_CPU_SET_CSR(mtvec, PAYLOAD_DEST);

   /*
    * Typically, we set a0 & a1 as arguements for the programing we are ging to
    * jump. Like a0 with hartid and a1 with a pointer to dtb so when we trap
    * they act as function arguments.
    * For example,
    * register int a0 asm("a0") = metal_cpu_get_current_hartid();
    * register unsigned long a1 asm("a1") = &dtb;
    */
   register int a0 asm("a0") = 0;
   register unsigned long a1 asm("a1") = 0;

   /* Before we trap, ensure a0 & a1 does spill/split for 'unimp' otherwise it is broken! */
   while(harts_checkin != num_harts) ;

   /* Trap away */
   asm volatile ("unimp" : : "r"(a0), "r"(a1));

   return 0;
}

/* Dummy to stop linker complaining! */
int main() {
   return 0;
}

