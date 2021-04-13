#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <pthread.h>

#include "utilities.c"

//FUNCTION PROTOTYPES
void signal_generator();
void signal_handler(int sig);
void handler(int sig);
void exit_handler(int sig);

//STRUCT FOR sharedCounters
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

//MAIN FUNCTION
int main() {

    int status;
    srand(time(NULL));

    //created shared memory and attach it to the parent process
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

    //create 3 processes for signal generating
    if ((signal_gen_proc_1 = fork()) == 0) {
        signal_generator();
    } else if ((signal_gen_proc_2 = fork()) == 0) {
        signal_generator();
    } else if ((signal_gen_proc_3 = fork()) == 0) {
        signal_generator();

        //create 2 processes for SIGUSR1 and 2 processes for SIGUSR2
    } else if ((signal_handler_1 = fork()) == 0) {
        signal_handler(SIGUSR1);
    } else if ((signal_handler_2 = fork()) == 0) {
        signal_handler(SIGUSR1);
    } else if ((signal_handler_3 = fork()) == 0) {
        signal_handler(SIGUSR2);
    } else if ((signal_handler_4 = fork()) == 0) {
        signal_handler(SIGUSR2);

        //create 1 process for reporting
    } else if ((reporter_proc = fork()) == 0) {
        //reporter();
    }
}

//FUNCTION THAT GENERATES EITHER SIGUSR1 OR SIGUSR2
void signal_generator() {
    int status;
    srand(time(NULL));
}

//FUNCTION WRAPPER FOR HANDLER
void signal_handler(int sig) {

    signal(SIGTERM, exit_handler);

    //unblock the signal that this process is handling so that it can receive the signal
    if (sig == SIGUSR1) unblock_sig1();
    if (sig == SIGUSR2) unblock_sig2();

    //set the handler function for the specified signal
    if (signal(sig, handler) == SIG_ERR) {
        fflush(stdout);
        printf("error setting handler!");
        return;
    }

    //wait for signals
    while (!sharedMem->kill_flag) {
        sleep(1);
    }

    shmdt(sharedMem);
    exit(EXIT_SUCCESS);
}

//FUNCTION FOR HANDLER
void handler(int sig) {

    fflush(stdout);

    if (sig == SIGUSR1) {
        //use locks
        pthread_mutex_lock(&(sharedMem->sig_1_received_lock));
        sharedMem->num_sig_1_received++;
        pthread_mutex_unlock(&(sharedMem->sig_1_received_lock));

    } else if (sig == SIGUSR2) {
        //use locks
        pthread_mutex_lock(&(sharedMem->sig_2_received_lock));
        sharedMem->num_sig_2_received++;
        pthread_mutex_unlock(&(sharedMem->sig_2_received_lock));
    }
}

//FUNCTION TO EXIT HANDLER
void exit_handler(int sig) {
    if (log_file != NULL) fclose(log_file);
    shmdt(sharedMem);
    exit(EXIT_SUCCESS);
}
