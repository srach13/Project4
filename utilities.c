#include "utilities.h"

void block_sigs() {
    sigset_t sigset;
    sigemptyset(&sigset); // initalize set to empty
    sigaddset(&sigset, SIGUSR1); // add SIGUSR1 to set
    sigaddset(&sigset, SIGUSR2); // add SIGUSR2 to
    sigaddset(&sigset, SIGINT); // add SIGUSR2 to
    sigprocmask(SIG_BLOCK, &sigset, NULL); // modify mask
}

void unblock_sig1() {
    sigset_t sigset;
    sigemptyset(&sigset); // initalize set to empty
    sigaddset(&sigset, SIGUSR1); // add SIGUSR1 to set
    sigprocmask(SIG_UNBLOCK, &sigset, NULL); // modify mask
};

void unblock_sig2() {
    sigset_t sigset;
    sigemptyset(&sigset); // initalize set to empty
    sigaddset(&sigset, SIGUSR2); // add SIGUSR2 to set
    sigprocmask(SIG_UNBLOCK, &sigset, NULL); // modify mask
};

// sleep for between 0.01 and 0.1 seconds
void interval_clock() {
    srand(time(NULL));
    int randint;
    randint = rand() % (100000 + 1 - 10000) + 10000; // random number between 0.01 and 0.01 seconds
    usleep(randint);
}

void init_locks(sharedCounters *sharedMem) {
    // initialize our mutex locks
    if (pthread_mutex_init(&(sharedMem->sig_1_received_lock), NULL) != 0) {
        fflush(stderr);
        fprintf(stderr, "error: failed to initialize sig_1_received_lock");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&(sharedMem->sig_2_received_lock), NULL) != 0) {
        fflush(stderr);
        fprintf(stderr, "error: failed to initialize sig_2_received_lock");
        exit(EXIT_FAILURE);
    };
    if (pthread_mutex_init(&(sharedMem->sig_1_sent_lock), NULL) != 0) {
        fflush(stderr);
        fprintf(stderr, "error: failed to initialize sig_1_sent_lock");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&(sharedMem->sig_2_sent_lock), NULL) != 0) {
        fflush(stderr);
        fprintf(stderr, "error: failed to initialize sig_2_sent_lock");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&(sharedMem->kill_lock), NULL) != 0) {
        fflush(stderr);
        fprintf(stderr, "error: failed to initialize sig_2_sent_lock");
        exit(EXIT_FAILURE);
    }
}