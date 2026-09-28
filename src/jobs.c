#include "jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

#define MAX_JOBS 10
#define MAX_JOB_PIDS 3

struct job {
    int   num;
    pid_t pids[MAX_JOB_PIDS];
    bool  done[MAX_JOB_PIDS];
    int   npids;
    char *cmdline;
    bool  active;
};

static struct job jobs[MAX_JOBS];   
static int next_job_num = 1;        

int jobs_full(void) {
    for (int i = 0; i < MAX_JOBS; i++)
        if (!jobs[i].active)
            return 0;
    return 1;
}

int add_job(pid_t *pids, int npids, char *cmdline) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].active) {
            struct job *j = &jobs[i];
            j->num = next_job_num++;
            j->npids = npids;
            for (int k = 0; k < npids; k++) {
                j->pids[k] = pids[k];
                j->done[k] = false;
            }
            j->cmdline = cmdline;
            j->active = true;
            printf("[%d] %d\n", j->num, (int)pids[npids - 1]);  // last cmd's PID 
            return j->num;
        }
    }
    free(cmdline);
    return -1;
}

// Prints "+ done" and frees the slot once every PID in the job is reaped
static void finish_if_done(struct job *j) {
    for (int k = 0; k < j->npids; k++)
        if (!j->done[k])
            return;
    printf("[%d] + done %s\n", j->num, j->cmdline);
    free(j->cmdline);
    j->cmdline = NULL;
    j->active = false;
}

void check_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        struct job *j = &jobs[i];
        if (!j->active)
            continue;
        for (int k = 0; k < j->npids; k++) {
            if (j->done[k])
                continue;
            int status;
            pid_t r = waitpid(j->pids[k], &status, WNOHANG);
            if (r == j->pids[k] || r == -1)   // finished, or already gone 
                j->done[k] = true;
        }
        finish_if_done(j);
    }
}

void print_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++)
        if (jobs[i].active)
            printf("[%d]+ %d %s\n", jobs[i].num,
                   (int)jobs[i].pids[jobs[i].npids - 1], jobs[i].cmdline);
}

// Blocks until every background job finishes (used by exit).
void wait_all_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        struct job *j = &jobs[i];
        if (!j->active)
            continue;
        for (int k = 0; k < j->npids; k++) {
            if (!j->done[k]) {
                int status;
                waitpid(j->pids[k], &status, 0);
                j->done[k] = true;
            }
        }
        finish_if_done(j);
    }
}