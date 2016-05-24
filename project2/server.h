// Struct for storing usage data
typedef struct {
    unsigned long size,resident,share,text,lib,data,dt;
} statm_t;

// Struct for arguments into pthread
struct arg_struct {
    void* arg1;
    void* arg2;
    void* arg3;
    FILE* arg4;
    int* arg5;
};

// Mutex for writing to log file
pthread_mutex_t lock;

// Signal handler
void sig_handler(int signo);