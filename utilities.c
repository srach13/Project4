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