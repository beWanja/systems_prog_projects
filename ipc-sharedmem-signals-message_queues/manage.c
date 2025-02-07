#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>

// Message structure for the message queue
typedef struct {
    long msg_type;
    pid_t pid;
    int perfect_num;
} Message;

// Global variables for cleanup
int shm_id, sem_id, msg_id;
SharedMemory *shared_mem;

// Semaphore operations
void sem_lock(int sem_id) {
    struct sembuf sb = {0, -1, 0};
    semop(sem_id, &sb, 1);
}

void sem_unlock(int sem_id) {
    struct sembuf sb = {0, 1, 0};
    semop(sem_id, &sb, 1);
}

// Signal handler to clean up resources
void handle_signal(int sig) {
    printf("Cleaning up IPC resources\n");
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    msgctl(msg_id, IPC_RMID, NULL);
    exit(0);
}


int main() {
    key_t shm_key = SHM_KEY, sem_key = SEM_KEY, msg_key = MSG_KEY;

    // Set up shared memory
    shm_id = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget");
        exit(1);
    }
    shared_mem = (SharedMemory *)shmat(shm_id, NULL, 0);
    if (shared_mem == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    // Set up semaphore
    sem_id = semget(sem_key, 1, IPC_CREAT | 0666);
    if (sem_id < 0) {
        perror("semget");
        exit(1);
    }
    semctl(sem_id, 0, SETVAL, 1);

    // Set up message queue
    msg_id = msgget(msg_key, IPC_CREAT | 0666);
    if (msg_id < 0) {
        perror("msgget");
        exit(1);
    }

    // Initialize shared memory
    memset(shared_mem, 0, sizeof(SharedMemory));
    shared_mem->manage_pid = getpid();

    // Signal handling
    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);
    signal(SIGHUP, handle_signal);

    // printf("Manage process started. PID: %d\n", getpid());
    
    // start off the counters and process id at 0
    for (int i = 0; i < MAX_PROCESSES; i++) {
        shared_mem->processes[i].pid = 0;
        shared_mem->processes[i].perfect_count = 0;
        shared_mem->processes[i].tested_count = 0;
        shared_mem->processes[i].skipped_count = 0;
    }

    Message msg;
    while (1) {
        // Wait for messages from compute
        if (msgrcv(msg_id, &msg, sizeof(Message) - sizeof(long), 0, 0) > 0) {
            sem_lock(sem_id);
            if (msg.msg_type == 2) { // Perfect number report using the message type 2
                // Add the reported perfect number to shared memory
                for (int i = 0; i < MAX_PERFECT_NUMS; i++) {
                    if (shared_mem->perfect_numbers[i] == 0) {
                        shared_mem->perfect_numbers[i] = msg.perfect_num;
                        printf("Perfect number: %d\n", msg.perfect_num);
                        break;
                    }
                }
            }
            sem_unlock(sem_id);
        }
    }


    return 0;
    
}
