#include "main.c"

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