#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

sharedCounters *sharedMem;
FILE *log_file = NULL;

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

    pid_t signal_gen_proc_1, signal_gen_proc_2, signal_gen_proc_3;
    pid_t signal_handler_1, signal_handler_2, signal_handler_3, signal_handler_4;
    pid_t reporter_proc;

    // create 3 processes for signal generating
    if ((signal_gen_proc_1 = fork()) == 0) {
        signal_generator();
    } else if ((signal_gen_proc_2 = fork()) == 0) {
        signal_generator();
    } else if ((signal_gen_proc_3 = fork()) == 0) {
        signal_generator();

    // create 2 processes for SIGUSR1 and 2 processes for SIGUSR2
    } else if ((signal_handler_1 = fork()) == 0) {
        signal_handler(SIGUSR1);
    } else if ((signal_handler_2 = fork()) == 0) {
        signal_handler(SIGUSR1);
    } else if ((signal_handler_3 = fork()) == 0) {
        signal_handler(SIGUSR2);
    } else if ((signal_handler_4 = fork()) == 0) {
        signal_handler(SIGUSR2);

}