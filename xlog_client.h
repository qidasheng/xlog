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
int paraNum; //参数个数
int client_socket = -1;
int thread_socket = -1;
FILE       *file_log;

struct process {
        int s[2];
};

struct process proc[100];

extern int  line_min_len;
extern int  line_max_len;
extern int  line_count_per;
extern char *server_ip;
extern long server_port;
extern int  server_retry_count;
extern int  server_retry_interval;
long line_count = 0;
long line_count_ok = 0;
long line_count_total = 0;

extern const char *conf_path;

extern  char log_file[LINE];
extern  char log_level[LINE];

struct conf_project project_arr[100];
static struct conf_public public_arr = {
        .line_min_len  = 0,
        .line_max_len = 0,
        .line_count_per   = 0,
        .server_addr   = "",
        .server_port   = 0,
        .server_retry_count   = 0,
        .server_retry_interval   = 0,
        .log_file   = "",
        .log_level   = "",
        .listen_addr   = "",
        .listen_port   = 0,
        .daemonize = ""
};

time_t t_start  = 0;

