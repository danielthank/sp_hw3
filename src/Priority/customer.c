#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "../error.h"

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

static void sig_normal(int signo) {
    complete[0]++;
    fprintf(logfile, "finish 0 %d\n", complete[0]);
}

static void sig_member(int signo) {
    complete[1]++;
    fprintf(logfile, "finish 1 %d\n", complete[1]);
}

static void sig_vip(int signo) {
    complete[2]++;
    fprintf(logfile, "finish 2 %d\n", complete[2]);
}

static void create_time(struct timespec* spec, long time) {
    spec->tv_sec = time / 1000000000L;
    spec->tv_nsec = time % 1000000000L;
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
        if (type == 0) continue;
        else if (type == 1)
            list[top].time = start * 1000000000L + 1000000000L;
        else
            list[top].time = start * 1000000000L + 300000000L;
        list[top].type = type;
        list[top].serial = cnt[type];
        list[top].isgenerate = 0;
        top++;
    }
    qsort(list, top, sizeof(task), cmp);

    struct sigaction newact;
    newact.sa_handler = sig_normal;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGINT, &newact, NULL);

    newact.sa_handler = sig_member;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGUSR1, &newact, NULL);

    newact.sa_handler = sig_vip;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGUSR2, &newact, NULL);

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
                    printf("ordinary\n");
                    fflush(stdout);
                    fsync(STDOUT_FILENO);
                    break;
                case 1:
                    kill(getppid(), SIGUSR1);
                    break;
                case 2:
                    kill(getppid(), SIGUSR2);
                    break;
            }
            fprintf(logfile, "send %d %d\n", list[i].type, generate[list[i].type]);
        }
        else {
            if (complete[list[i].type] < list[i].serial) {
                fprintf(logfile, "timeout\n");
                break;
            }
        }
    }
    return 0;
}
