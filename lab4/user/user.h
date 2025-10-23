struct stat;
struct rtcdate;
struct sysinfo;
typedef struct __lock_t {
  int ticket;
  int turn;
} lock_t;

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);

// Lab 1: Syscalls
int trace(int);
int sysinfo(struct sysinfo *);

// Lab 2: Page Tables
int pgaccess(void *base, int len, void *mask);
// usyscall region
int ugetpid(void);

// Lab 3: Traps
int sigalarm(int ticks, void (*handler)());
int sigreturn(void);

// Lab 4: Kernel Threads
int clone(void (*fn)(uint64, uint64), uint64 arg1, uint64 arg2, void *stack);
int join(void **stack, int *retval);

// Lab 6: Net
int connect(uint32, uint16, uint16);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...);
void printf(const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);

// uthread.c (lab 4)
int thread_create(void (*fn)(uint64, uint64), uint64 arg1, uint64 arg2);
int thread_join(int *retval);
void lock_init(lock_t *lock);
void lock_acquire(lock_t *lock);
void lock_release(lock_t *lock);
