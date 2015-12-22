#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "../utility.h"

#define SIGUSR3 SIGWINCH

FILE *data, *logfile;

typedef struct {
    long time;
    int type;
    int serial;
    char isgenerate;
} task;

task list[10000];
int top, cnt[3], generate[3], complete[3];

static int cmp(const void *a, const void *b) {
    return ((task*)a)->time > ((task*)b)->time;
}

static void sig_check(int signo) {
    int sig;
    if (signo == SIGUSR1) sig = 0;
    else if (signo == SIGUSR2) sig = 1;
    else sig = 2;
    complete[sig]++;
    fprintf(logfile, "finish %d %d\n", sig, complete[sig]);
}

int main(int argc, char *argv[]) {
    logfile = fopen("customer_log", "w");
    if (argc != 2)
        err_quit("Command parameter error");
    data = fopen(argv[1], "r");
    int type;
    float start;
    list[top++].time = 0;
    while(fscanf(data, "%d%f", &type, &start) > 0) {
        //fprintf(stderr, "%d %f\n", type, start);
        cnt[type]++;
        list[top].time = start * 1000000000L;
        list[top].type = type;
        list[top].serial = cnt[type];
        list[top].isgenerate = 1;
        top++;
        if (type == 0)
            list[top].time = start * 1000000000L + 2000000000L;
        else if (type == 1)
            list[top].time = start * 1000000000L + 3000000000L;
        else
            list[top].time = start * 1000000000L + 300000000L;
        list[top].type = type;
        list[top].serial = cnt[type];
        list[top].isgenerate = 0;
        top++;
    }
    qsort(list, top, sizeof(task), cmp);

    struct sigaction newact;
    newact.sa_handler = sig_check;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGUSR1, &newact, NULL);
    sigaction(SIGUSR2, &newact, NULL);
    sigaction(SIGUSR3, &newact, NULL);

    struct timespec req, rem;
    create_time(&req, list[0].time);
    create_time(&rem, 0);
    nanosleep(&req, &rem);
    for (int i=1; i<top; i++) {
        //fprintf(stderr, "%d %d %d\n", cnt[0], cnt[1], cnt[2]);
        //fprintf(stderr, "%d %d %d\n", complete[0], complete[1], complete[2]);
        //fprintf(stderr, "i.time: %ld\n", list[i].time);
        create_time(&req, list[i].time-list[i-1].time);
        create_time(&rem, 0);
        while(req.tv_sec != 0 || req.tv_nsec != 0) {
            nanosleep(&req, &rem);
            req = rem;
            create_time(&rem, 0);
        }
        if (complete[0] + complete[1] + complete[2] == cnt[0] + cnt[1] + cnt[2]) break;

        if (list[i].isgenerate) {
            generate[list[i].type]++;
            switch(list[i].type) {
                case 0:
                    kill(getppid(), SIGUSR1);
                    break;
                case 1:
                    kill(getppid(), SIGUSR2);
                    break;
                case 2:
                    kill(getppid(), SIGUSR3);
                    break;
            }
            fprintf(logfile, "send %d %d\n", list[i].type, generate[list[i].type]);
        }
        else {
            if (complete[list[i].type] < list[i].serial) {
                fprintf(logfile, "timeout %d %d\n", list[i].type, list[i].serial);
            }
        }
    }
    printf("terminate\n");
    fflush(stdout);
    fsync(STDOUT_FILENO);
    return 0;
}
