#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

int  jobs_full(void);
int  add_job(pid_t *pids, int npids, char *cmdline);  // takes ownership of cmdline 
void check_jobs(void);
void print_jobs(void);
void wait_all_jobs(void);

#endif