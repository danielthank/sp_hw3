#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>
#include "../error.h"

#define SIGUSR3 SIGWINCH

char buf[4096];
long deadline[3], time_now, remain[3];
int processing, pending[3], cnt[3], sig_table[3];
pid_t pid;
FILE *logfile;

static void create_time(struct timespec* spec, long time) {
    spec->tv_sec = time / 1000000000L;
    spec->tv_nsec = time % 1000000000L;
}

static long create_long(struct timespec* spec) {
    return spec->tv_sec * 1000000000L + spec->tv_nsec;
}

static void sig_check(int signo) {
    int sig;
    if (signo == 0) {
        sig = 3;
    }
    if (signo == SIGUSR1) {
        sig = 0;
        deadline[0] = time_now + 2000000000L;
        remain[0] = 500000000L;
    }
    else if (signo == SIGUSR2) {
        sig = 1;
        deadline[1] = time_now + 3000000000L;
        remain[1] = 1000000000L;
    }
    else if (signo == SIGUSR3) {
        sig = 2;
        deadline[2] = time_now + 300000000L;
        remain[2] = 200000000L;
    }
    if (sig < 3) pending[sig] = 1;
    long min = (long)1e18;
    processing = 3;
    for (int i=0; i<3; i++) {
        if (remain[i] != 0 && deadline[i] < min) {
            min = deadline[i];
            processing = i;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        err_quit("Command parameter error");
    int fd[2];
    sigset_t newmask, oldmask;
    struct sigaction newact;

    newact.sa_handler = sig_check;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGUSR1, &newact, NULL);
    sigaction(SIGUSR2, &newact, NULL);
    sigaction(SIGUSR3, &newact, NULL);

    pipe(fd);

    if ((pid = fork()) < 0)
        err_sys("Fork error");
    else if (pid == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        if (execl("customer_EDF", "./customer_EDF", argv[1], (char*)0) < 0)
            err_sys("execl error");
    }
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigaddset(&newmask, SIGUSR2);
    sigaddset(&newmask, SIGUSR3);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);
    dup2(fd[0], STDIN_FILENO);
    close(fd[1]);

    logfile = fopen("bidding_system_log", "w");
    //dup2(STDERR_FILENO, fileno(logfile));
    processing = 3;
    time_now = 0;
    sig_table[0] = SIGUSR1;
    sig_table[1] = SIGUSR2;
    sig_table[2] = SIGUSR3;
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
    while (1) {
        while (processing < 3) {
            int save_id = processing;
            if (pending[save_id]) {
                pending[save_id] = 0;
                cnt[save_id]++;
                fprintf(logfile, "receive %d %d\n", save_id, cnt[save_id]);
            }
            struct timespec req, rem;
            create_time(&req, remain[save_id]);
            create_time(&rem, 0);
            nanosleep(&req, &rem);
            time_now += create_long(&req) - create_long(&rem);
            remain[save_id] = create_long(&rem);
            if (remain[save_id] == 0) {
                kill(pid, sig_table[save_id]);
                fprintf(logfile, "finish %d %d\n", save_id, cnt[save_id]);
                sig_check(0);
            }
        }
        int n, remain = 10;
        fd_set listen;
        FD_ZERO(&listen);
        FD_SET(STDIN_FILENO, &listen);
        int ok = select(STDIN_FILENO+1, &listen, NULL, NULL, NULL);
        if (ok > 0) {
reread:
            n = read(STDIN_FILENO, buf, remain);
            buf[n] = 0;
            //fprintf(stderr, "buf: %s\n", buf);
            if (n == remain) {
                fprintf(logfile, "terminate\n");
                break;
            }
            else if (n > 0) {
                remain -= n;
                goto reread;
            }
            else if (n < 0 && errno == EINTR) {
                goto reread;
            }
            else break;
        }
    }

    while ((pid = waitpid(-1, NULL, 0)))
        if (errno == ECHILD)
            break;
    return 0;
}
