#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>

// Message structure for message q
typedef struct {
    long msg_type;
    pid_t pid;
    int perfect_num;
} Message;

// Semaphore operations
void sem_lock(int sem_id) {
    struct sembuf sb = {0, -1, 0};
    semop(sem_id, &sb, 1);
}

void sem_unlock(int sem_id) {
    struct sembuf sb = {0, 1, 0};
    semop(sem_id, &sb, 1);
}

// Function to checkfor perfect number
// based on the formula for calculating the sum of divisors of a number
int is_perfect(int n) {
    if (n < 2) return 0;

    int sum = 1; 
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            if (i * i == n) {
                sum += i;
            } else {
                sum += i + n / i;
            }
        }
    }
    return sum == n;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s START\n", argv[0]);
        exit(1);
    }

    int start = atoi(argv[1]);

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
    
    // Access message queue
    int msg_id = msgget(MSG_KEY, 0666);
    if (msg_id == -1) {
        perror("msgget failed");
        exit(1);
    }

    // Get process index
    int process_index = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (shared_mem->processes[i].pid == 0) {
            shared_mem->processes[i].pid = getpid();
            process_index = i;
            break;
        }
    }
    if (process_index == -1) {
        fprintf(stderr, "No available slots for processes\n");
        exit(1);
    }

    // printf("Compute process started. PID: %d\n", getpid());

    // Update statistics
    while (1) {
        for (int i = start; i < start + 1000000; i++) {
            sem_lock(shm_id);
            if ((shared_mem->bitmap[i / 8] & (1 << (i % 8))) == 0) {
                shared_mem->bitmap[i / 8] |= (1 << (i % 8));
                shared_mem->processes[process_index].tested_count++;
                sem_unlock(shm_id);

                if (is_perfect(i)) {
                    shared_mem->processes[process_index].perfect_count++; 
                    // Send perfect number to manage
                    Message msg;
                    msg.msg_type = 2;       // will mark perfect numbers msg type
                    msg.pid = getpid();     
                    msg.perfect_num = i;   

                    if (msgsnd(msg_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) { 
                        perror("msgsnd failed");
                        exit(1);
                    }
                }
            } else {
                shared_mem->processes[process_index].skipped_count++;
                sem_unlock(shm_id);
            }
        }

        break;
    }

    // printf("Compute process PID %d finished.\n", getpid());
    return 0;
}
