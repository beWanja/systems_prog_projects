#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

void print_report() {
    // Access shared memory
    int shm_id = shmget(SHM_KEY, sizeof(SharedMemory), 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }
    SharedMemory *shared_mem = (SharedMemory *)shmat(shm_id, NULL, 0);
    if (shared_mem == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    int total_found = 0;
    int total_tested = 0;
    int total_skipped = 0;
    printf("Perfect Numbers Found:\n");

    // Print perfect numbers
    for (int i = 0; i < MAX_PERFECT_NUMS; i++) {
        if (shared_mem->perfect_numbers[i] != 0) {
            printf("%d ", shared_mem->perfect_numbers[i]);
            total_found++;
        }
    }
    printf("\n");
    
    // Print individual process stats
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (shared_mem->processes[i].pid != 0) {
            printf("pid(%d): found: %d, tested: %d, skipped: %d\n",
                   shared_mem->processes[i].pid,
                   shared_mem->processes[i].perfect_count,
                   shared_mem->processes[i].tested_count,
                   shared_mem->processes[i].skipped_count);

            total_tested += shared_mem->processes[i].tested_count;
            total_skipped += shared_mem->processes[i].skipped_count;
        }
    }

    // Print summary statistics
    printf("Statistics:\n");
    printf("Total found:   %d\n", total_found);
    printf("Total tested:  %d\n", total_tested);
    printf("Total skipped: %d\n", total_skipped);
}


// Main function
int main(int argc, char *argv[]) {
    int shutdown = 0;
    if (argc == 2 && strcmp(argv[1], "-k") == 0) {
        shutdown = 1; //mark the -k flag
    }
    
    print_report();

    key_t shm_key = SHM_KEY;
    int shm_id = shmget(shm_key, sizeof(SharedMemory), 0666);
    SharedMemory *shared_mem = (SharedMemory *)shmat(shm_id, NULL, 0);

    if (shutdown) {
        kill(shared_mem->manage_pid, SIGINT); // send the interrupt signal to the manage process using kill function
    }

    return 0;
}
