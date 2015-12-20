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

#define MAX_QUEUE 10000

char buf[4096];
long normal_queue[MAX_QUEUE];
int top = 0, tail = 0, cnt[3];
pid_t pid;
FILE *logfile;

static void sig_member(int signo) {
    cnt[1]++;
    fprintf(logfile, "receive 1 %d\n", cnt[1]);
    struct timespec req, rem;
    req.tv_sec = 0;
    req.tv_nsec = 500000000L;
    while (1) {
        nanosleep(&req, &rem);
        if (rem.tv_nsec == 0) break;
        req = rem;
        rem.tv_sec = 0;
        rem.tv_nsec = 0;
    }
    kill(pid, SIGUSR1);
    fprintf(logfile, "finish 1 %d\n", cnt[1]);
}

static void sig_vip(int signo) {
    cnt[2]++;
    fprintf(logfile, "receive 2 %d\n", cnt[2]);
    struct timespec req, rem;
    req.tv_sec = 0;
    req.tv_nsec = 200000000L;
    nanosleep(&req, &rem);
    kill(pid, SIGUSR2);
    fprintf(logfile, "finish 2 %d\n", cnt[2]);
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        err_quit("Command parameter error");
    int fd[2];
    sigset_t newmask, oldmask;
    struct sigaction newact, oldact_usr1, oldact_usr2;

    newact.sa_handler = sig_member;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGUSR1, &newact, &oldact_usr1);

    newact.sa_handler = sig_vip;
    sigemptyset(&newact.sa_mask);
    sigaddset(&newact.sa_mask, SIGUSR1);
    newact.sa_flags = 0;
    sigaction(SIGUSR2, &newact, &oldact_usr2);


    pipe(fd);

    if ((pid = fork()) < 0)
        err_sys("Fork error");
    else if (pid == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        if (execl("customer", "./customer", argv[1], (char*)0) < 0)
            err_sys("execl error");
    }
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigaddset(&newmask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);
    dup2(fd[0], STDIN_FILENO);
    close(fd[1]);

    logfile = fopen("bidding_system_log", "w");
    //dup2(STDERR_FILENO, fileno(logfile));
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
    while (1) {
        while (top != tail) {
            struct timespec req, rem;
            req.tv_sec = normal_queue[tail]/1000000000L;
            req.tv_nsec = normal_queue[tail]%1000000000L;
            if (nanosleep(&req, &rem) == -1 && errno == EINTR) {
                normal_queue[tail] = rem.tv_sec * 1000000000 + req.tv_nsec;
            }
            else {
                kill(pid, SIGINT);
                tail++;
                fprintf(logfile, "finish 0 %d\n", tail);
            }
        }
        int n, remain = 9;
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
                cnt[0]++;
                fprintf(logfile, "receive 0 %d\n",cnt[0]);
                normal_queue[top++] = 1000000000L;
                top %= MAX_QUEUE;
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

    fprintf(logfile, "terminate\n");

    while ((pid = waitpid(-1, NULL, 0)))
        if (errno == ECHILD)
            break;
    return 0;
}
