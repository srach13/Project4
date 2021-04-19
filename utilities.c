#include "main.c"

void unblock_sig1() {
    sigset_t sigset;
    sigemptyset(&sigset); //initialize set to empty
    sigaddset(&sigset, SIGUSR1); //add SIGUSR1 to set
    sigprocmask(SIG_UNBLOCK, &sigset, NULL); //modify mask
};

void unblock_sig2() {
    sigset_t sigset;
    sigemptyset(&sigset); //initialize set to empty
    sigaddset(&sigset, SIGUSR2); //add SIGUSR2 to set
    sigprocmask(SIG_UNBLOCK, &sigset, NULL); //modify mask
};

void init_locks(sharedCounters *sharedMem) {
    // initialize mutex locks
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