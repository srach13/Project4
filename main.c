#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/time.h>

#include "utilities.c"

void signal_generator();
void signal_handler(int sig);
void handler(int sig);
void exit_handler(int sig);
void reporter();

sharedCounters *sharedMem;
FILE *log_file = NULL;

int mode = 0; // mode = 1 for 30 second run, mode = 0 for 100,000 signal run

int sig_1_handled = 0, sig_2_handled = 0; // bad ways for the reporter proc to keep track of what's been sent

int main() {
    int status;
    srand(time(NULL));

    // created shared memory and attach it to the parent process
    // since we attached to the parent and fork, the children will be attached automatically
    int shm_id = shmget(IPC_PRIVATE, sizeof(sharedCounters), IPC_CREAT | 0666);
    assert(shm_id >= 0);
    sharedMem = (sharedCounters *) shmat(shm_id, NULL, 0);
    sharedMem->num_sig_1_sent = 0;
    sharedMem->num_sig_2_sent = 0;
    sharedMem->num_sig_1_received = 0;
    sharedMem->num_sig_2_received = 0;
    sharedMem->kill_flag = 0;

    init_locks(sharedMem);

    block_sigs(); // block signals so that we can unblock wanted signals in the right processes later

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

        // create 4 processes for signal handling, 2 for SIGUSR1 and 2 for SIGUSR2
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

        // stay in parent process
    } else {
        // run for 30 secs or 100,000 signals
        if (mode) {
            sleep(30);
        } else {
            while (1) {
                pthread_mutex_lock(&(sharedMem->sig_1_received_lock));
                pthread_mutex_lock(&(sharedMem->sig_2_received_lock));

                if (sharedMem->num_sig_2_received + sharedMem->num_sig_1_received >= 100000) break;

                if (sharedMem->num_sig_2_received > 0 && sharedMem->num_sig_2_received + sharedMem->num_sig_1_received % 1000 == 0) {
                    fflush(stdout);
                    printf("1000 sigs\n");
                }

                pthread_mutex_unlock(&(sharedMem->sig_1_received_lock));
                pthread_mutex_unlock(&(sharedMem->sig_2_received_lock));
                sleep(1);
            }
        }

        // end all the children processes
        pthread_mutex_lock(&(sharedMem->kill_lock));
        sharedMem->kill_flag = 1;
        pthread_mutex_unlock(&(sharedMem->kill_lock));

        kill(signal_gen_proc_1, SIGTERM);
        kill(signal_gen_proc_2, SIGTERM);
        kill(signal_gen_proc_3, SIGTERM);
        kill(signal_handler_1, SIGTERM);
        kill(signal_handler_2, SIGTERM);
        kill(signal_handler_3, SIGTERM);
        kill(signal_handler_4, SIGTERM);
        kill(reporter_proc, SIGTERM);

        while (wait(&status) > 0);

        shmdt(sharedMem);
        shmctl(shm_id, IPC_RMID, NULL);

        return EXIT_SUCCESS;
    }

    shmdt(sharedMem);
    return EXIT_SUCCESS;
}

// randomly generate either SIGUSR1 or SIGUSR2 and then sleep for 0.01 to 0.1 seconds
void signal_generator() {
    int status;
    srand(time(NULL));

    signal(SIGTERM, exit_handler);

    while (1) {

        interval_clock(); // sleep before next signal
        if (rand() % 2) {
            // critical section - lock and then send signal and increment counter
            pthread_mutex_lock(&(sharedMem->sig_1_sent_lock));
            // send signal to every process in this process group
            if ((status = kill(0, SIGUSR1)) < 0) {
                fflush(stderr);
                fprintf(stderr, "error: generating SIGUSR1: %i\n", status);
                exit(EXIT_FAILURE);
            }
            sharedMem->num_sig_1_sent += 1;
            pthread_mutex_unlock(&(sharedMem->sig_1_sent_lock));

        } else {
            // critical section - lock and then send signal and increment counter
            pthread_mutex_lock(&(sharedMem->sig_2_sent_lock));
            // send signal to every process in this process group
            if ((status = kill(0, SIGUSR2)) < 0) {
                fflush(stderr);
                fprintf(stderr, "error: generating SIGUSR2: %i\n", status);
                exit(EXIT_FAILURE);
            }
            sharedMem->num_sig_2_sent += 1;
            pthread_mutex_unlock(&(sharedMem->sig_2_sent_lock));
        }
    }
}

// essentially a wrapper for handler
void signal_handler(int sig) {

    signal(SIGTERM, exit_handler);

    // unblock the signal that this process is handling so that it can receive the signal
    if (sig == SIGUSR1) unblock_sig1();
    if (sig == SIGUSR2) unblock_sig2();

    // set the handler function for the specified signal
    if (signal(sig, handler) == SIG_ERR) {
        fflush(stdout);
        printf("error setting handler!");
        return;
    }

    // wait in a loop for signals
    while (!sharedMem->kill_flag) {
        sleep(1);
    }

    shmdt(sharedMem);
    exit(EXIT_SUCCESS);
}

void handler(int sig) {
    fflush(stdout);
    if (sig == SIGUSR1) {
        // critical section, so use locks
        pthread_mutex_lock(&(sharedMem->sig_1_received_lock));
        sharedMem->num_sig_1_received++;
        pthread_mutex_unlock(&(sharedMem->sig_1_received_lock));
    } else if (sig == SIGUSR2) {
        // critical section, so use locks
        pthread_mutex_lock(&(sharedMem->sig_2_received_lock));
        sharedMem->num_sig_2_received++;
        pthread_mutex_unlock(&(sharedMem->sig_2_received_lock));
    }
}

void reporter_handler(int sig) {
    if (sig == SIGUSR1)
        sig_1_handled++;
    else if (sig == SIGUSR2)
        sig_2_handled++;
}

// log signal events to keep track of progression
void reporter() {
    log_file = fopen("log.txt", "w");
    if (log_file == NULL) {
        fflush(stderr);
        perror("error: could not open log file\n");
        exit(EXIT_FAILURE);
    }

    time_t rawtime;
    struct tm * timeinfo;
    char * timestr;

    struct timeval start, stop;

    // the reporter process should handle both SIGUSR1 and SIGUSR2, so unblock them
    unblock_sig1();
    unblock_sig2();

    // add handler for signals
    if (signal(SIGUSR1, reporter_handler) == SIG_ERR) {
        fflush(stdout);
        printf("error setting handler!");
    }
    if (signal(SIGUSR2, reporter_handler) == SIG_ERR) {
        fflush(stdout);
        printf("error setting handler!");
    }

    if (signal(SIGTERM, exit_handler) == SIG_ERR) {
        fflush(stdout);
        printf("error setting handler!");
    }

    gettimeofday(&start, NULL);
    while (1) {

        // every 10 signals, exclude initial condition of 0 signals
        if (sig_1_handled > 1 && (sig_1_handled + sig_2_handled) % 10 == 0) {
            gettimeofday(&stop, NULL);
            double diff = (stop.tv_sec + (1.0/1000000) * stop.tv_usec) - (start.tv_sec + (1.0/1000000) * start.tv_usec);

            // grab a nicely formatted system time
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            timestr = asctime(timeinfo);
            timestr[strlen(timestr) - 1] = '\0';

            pthread_mutex_lock(&(sharedMem->sig_1_received_lock));
            pthread_mutex_lock(&(sharedMem->sig_2_received_lock));
            pthread_mutex_lock(&(sharedMem->sig_1_sent_lock));
            pthread_mutex_lock(&(sharedMem->sig_2_sent_lock));

            fflush(log_file);
            fprintf(log_file, "[%s] %i SIGUSR1 sent, %i SIRUSR2 sent, %i SIGUSR1 received, %i SIGUSR2 received, "
                              "avg %.3lf sec between SIGUSR1, avg %.3lf sec between SIGUSR2\n",
                    timestr, sharedMem->num_sig_1_sent, sharedMem->num_sig_2_sent,
                    sharedMem->num_sig_1_received, sharedMem->num_sig_2_received,
                    diff/sharedMem->num_sig_1_received, diff/sharedMem->num_sig_2_received);

            pthread_mutex_unlock(&(sharedMem->sig_1_received_lock));
            pthread_mutex_unlock(&(sharedMem->sig_2_received_lock));
            pthread_mutex_unlock(&(sharedMem->sig_1_sent_lock));
            pthread_mutex_unlock(&(sharedMem->sig_2_sent_lock));
        }

        sleep(1);
    }
}

void exit_handler(int sig) {
    if (log_file != NULL) fclose(log_file);
    shmdt(sharedMem);
    exit(EXIT_SUCCESS);
}
