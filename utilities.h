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