#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>


// FUNCTION PROTOTYPES
void interval_clock();
void signal_generator();

// STRUCT FOR sharedCounters
typedef struct sharedCounters {
    int num_sig_1_sent;
    int num_sig_2_sent;
    int num_sig_1_received;
    int num_sig_2_received;
    int kill_flag;
    pthread_mutex_t sig_1_sent_lock;
    pthread_mutex_t sig_2_sent_lock;
    pthread_mutex_t sig_1_received_lock;
    pthread_mutex_t sig_2_received_lock;
    pthread_mutex_t kill_lock;
} sharedCounters;


sharedCounters *sharedMem;
FILE *log_file = NULL;

int mode = 0;
int sig_1_handled = 0, sig_2_handled = 0;

// MAIN FUNCTION
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

        // create 1 process for reporting
    } else if ((reporter_proc = fork()) == 0) {
        reporter();
    }
}

// FUNCTION TO GET INTERVAL FOR SLEEP B/T 0.01 AND 0.1 SECONDS
void interval_clock() {
    srand(time(NULL));
    int randint;
    randint = rand() % (100000 + 1 - 10000) + 10000; // get random number between 0.01 and 0.01 seconds
    usleep(randint);
}

// FUNCTION THAT GENERATES EITHER SIGUSR1 OR SIGUSR2
void signal_generator() {
    int status;
    srand(time(NULL));
}


