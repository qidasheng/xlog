#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <string.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <ctype.h>
//#include <glib.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <syslog.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define BUFFER  4096
#define LINE    4096
#define _(s) s
#ifndef PR_SET_NAME
#define PR_SET_NAME 15
#endif

#ifndef PR_GET_NAME
#define PR_GET_NAME 16
#endif

#ifndef ALLOC_H_INCLUDED
#define ALLOC_H_INCLUDED
#endif


extern char **environ;
static char *arg_start;
static char *arg_end;
static char *env_start;



typedef struct conf_public {
        int  line_min_len;
        int  line_max_len;
        int  line_count_per;
        char server_addr[LINE];
        long server_port;
        int  server_retry_count;
        int  server_retry_interval;
        char log_file[LINE];
        char log_level[LINE];
        char listen_addr[LINE];
        long listen_port;
        char daemonize[LINE];
} conf_public;


typedef struct conf_project {
        char name[LINE];
        int  from_begin;
        char path[LINE];
        char type[LINE];
        long  pid;
        long count;
        long count_ok;
        long count_total;
        struct conf_public config;
} conf_project;

static void show_help(void) {
    char *b = "--------------------------------------------------------------------------------------------------\n"
            "This is free software, and you are welcome to modify and redistribute it under the BSD License\n"
            "\n"
            "-c <config_addr>  config address\n"
            "-d            run as a daemon\n"
            "-h            print this help and exit\n\n"
            "\n"
            "--------------------------------------------------------------------------------------------------\n"
            "\n";
    fprintf(stderr, b, strlen(b));
}

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *arry;
};

static int sem_id = 0;

static int set_semvalue();
static void del_semvalue();
static int semaphore_p();
static int semaphore_v();

static int set_semvalue()
{
	//用于初始化信号量，在使用信号量前必须这样做
	union semun sem_union;

	sem_union.val = 1;
	if(semctl(sem_id, 0, SETVAL, sem_union) == -1)
		return 0;
	return 1;
}

static void del_semvalue()
{
	//删除信号量
	union semun sem_union;

	if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
		fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p()
{
	//对信号量做减1操作，即等待P（sv）
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;//P()
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1)
	{
		fprintf(stderr, "semaphore_p failed\n");
		return 0;
	}
	return 1;
}

static int semaphore_v()
{
	//这是一个释放操作，它使信号量变为可用，即发送信号V（sv）
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;//V()
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1)
	{
		fprintf(stderr, "semaphore_v failed\n");
		return 0;
	}
	return 1;
}

