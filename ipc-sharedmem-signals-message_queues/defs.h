/*
This header file defines the constants, structures, and keys shared between manage.c, compute.c and report.c 
*/
#ifndef DEFS_H
#define DEFS_H

#include <sys/types.h>

//Arbitrary keys for shared memory, semaphores and message queues respectively
#define SHM_KEY 33682   
#define SEM_KEY 44444
#define MSG_KEY 55555

#define MAX_PROCESSES 20
#define MAX_PERFECT_NUMS 20
#define BITMAP_SIZE (1 << 22)  // 2^22 bytes

// Process structure
typedef struct {
    pid_t pid;
    int perfect_count;
    int tested_count;
    int skipped_count;
} Process;

// Shared memory structure
typedef struct {
    unsigned char bitmap[BITMAP_SIZE];
    int perfect_numbers[MAX_PERFECT_NUMS];
    Process processes[MAX_PROCESSES];
    int manage_pid;
    Process terminated_stats;
} SharedMemory;

#endif
