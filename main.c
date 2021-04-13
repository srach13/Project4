#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

int main() {

    int status;
    srand(time(NULL));

    // created shared memory and attach it to the parent process
    int shm_id = shmget(IPC_PRIVATE, sizeof(sharedCounters), IPC_CREAT | 0666);
    assert(shm_id >= 0);
    sharedMem = (sharedCounters *) shmat(shm_id, NULL, 0);
    sharedMem->num_sig_1_sent = 0;
    sharedMem->num_sig_2_sent = 0;
    sharedMem->num_sig_1_received = 0;
    sharedMem->num_sig_2_received = 0;
    sharedMem->kill_flag = 0;

}