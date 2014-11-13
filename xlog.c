#include "xlog.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

int paraNum; //参数个数
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
long line_count_total = 0;
long line_count_ignore = 0;

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

extern void sendMsg(char *ip, unsigned int port, char *msg, int index, conf_project *c_shmaddr);

long  xFile(FILE *fp, char *msg, int mode) {
	long log_offset = 0;
	char line[1024]; 
	if (msg != NULL) {
		if ( mode = 1) {
			fseek(fp, 0, SEEK_SET);
			fwrite(msg, sizeof(msg), strlen(msg), fp);	
		} else {
			fwrite(msg, sizeof(msg), strlen(msg), fp);	
		}
	} else {
		fseek(fp, 0, SEEK_SET);
                fgets(line, 1024, fp);
		log_offset = atoi(line);
		return log_offset;
	}
	return 0;
}



void *
debug_malloc(size_t size, const char *file, int line, const char *func)
{
        void *p;
        p = malloc(size);
        //printf("%s:%d:%s:malloc(%ld): p=0x%lx\n", file, line, func, size, (unsigned long)p);
        return p;
}
/*
#define malloc(s) debug_malloc(s, __FILE__, __LINE__, __func__)
#define free(p)  do {                                                   \
        printf("%s:%d:%s:free(0x%lx)\n", __FILE__, __LINE__,            \
            __func__, (unsigned long)p);                                \
} while (0)
*/

void sig_handler( int sig)
{
       if(sig == SIGUSR1){
	    get_conf();
       }
}




unsigned long long get_datetime() {
    unsigned long long datetime = 0;
    char *datetimestr;
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    sprintf(datetimestr,"%d%d%d%d%d%d", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    datetime = atoll(datetimestr);
    return datetime;
}


time_t get_timestamp() {
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    timep = mktime(p);
    return timep;
}



/* macro for easily allocating memory */
#define MALLOC_STRUCT(target, size)                 \
do {                                                \
	target = malloc(sizeof * target * size);         \
	if (target) {                                    \
		size_t i;                                     \
		for (i = 0; i < size; i++) {                  \
			target[i] = malloc(sizeof * target[i]);	 \
		}                                             \
	}                                                \
} while (0);


void
error_handler (const char *func, char *file, int line, char *msg)
{
   fprintf (stderr, "\nxlog_format - version %s - %s %s\n", "0.1",
            __DATE__, __TIME__);
   fprintf (stderr, "\nAn error has occurred");
   fprintf (stderr, "\nError occured at: %s - %s - %d", file, func, line);
   fprintf (stderr, "\nMessage: %s\n\n", msg);
   exit (EXIT_FAILURE);
}


void add_log(char *msg) {
	printf("写入日志：%s\n", msg);
	sprintf(msg,"%s\n",msg);
	if (msg != NULL) {
		fwrite(msg, sizeof(msg), strlen(msg), file_log);	
	}
}


char *
alloc_string (const char *str)
{
   char *new = malloc (strlen (str) + 1);
   if (new == NULL)
      error_handler (__PRETTY_FUNCTION__,
                     __FILE__, __LINE__, "Unable to allocate memory");
   strcpy (new, str);
   return new;
}







char *rtrim_str (char *str)
{
   char *p;
   if (!str)
      return NULL;
   if (!*str)
      return str;
   for (p = str + strlen (str) - 1; (p >= str) && isspace (*p); --p);
   p[1] = '\0';
   return str;
}

char *ltrim_str (char *str){
   char *p;
   if (!str)
      return NULL;
   if (!*str)
      return str;
   for (p = str ; (p <= str) && isspace (*p); ++p);
   str = p;
   return str;

}

char *trim_str (char *str){
   return ltrim_str(rtrim_str(str));
}



static size_t filesize(const char *filename)
{
    struct stat sb;
    if (!stat(filename, &sb)) return sb.st_size;
    return 0;
}



int is_ip_start(const char *buf) {
        char *p;
	char *tmp;
        p = (char *)malloc(strlen(buf) + 1);
        strcpy(p, buf);
        tmp = p;
        //printf("%s# # #\n",p);
        int ip_min_len = 7;
        int ip_max_len = 15;
        int dot_count = 0;
        int step  = 1;
        int len = 0;
        while (*p != '\0' || len <= ip_max_len) {
                //printf("-> %c \n", *p);
                if (isspace(*p)) {
                        break;
                }
                if (step == 1 &&  !isdigit(*p)) {
                        break;
                }
                if (step > 1 && (!isdigit(*p) && *p != '.')) {
                        break;
                }

                if (step > 4) {
                        break;
                }
                step++;
                if (*p == '.') {
                        dot_count = dot_count + 1;
                        step = 1;
                }

                //printf("%d=%d\n", dot_count,step);
                p++;
                len++;

        }
	free(tmp);
	tmp = NULL;
        if (dot_count != 3) {
                return 0;
        }
        //printf("1#%d\n", dot_count);
        return 1;
}



static void xlog_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("xlog_daemon", LOG_PID, LOG_DAEMON);
}


//分析日志文件
int listen_log(conf_public *public,conf_project *project, int index, conf_project *c_shmaddr) {
	//printf("pid is :%ld\n", project[index].pid);
	int count = 0;
	char       buffer[BUFFER];
	int        line_max = 32 * 1024;
	char       buf[line_max];
	char       fork_buf[line_max];
	size_t     osize, nsize;
	FILE       *str, *offset_log;
	char *filename;
	filename = project[index].path;
       
	if (!(str = fopen(filename, "r"))) {
		fprintf(stderr, _("Cannot open web log  \"%s\" for read\n"), filename);
		exit(1);
	}

	char *offset_log_path = "/tmp/xlog_offset_zt";
        if (!(offset_log = fopen(offset_log_path, "rw+"))) {
                fprintf(stderr, _("Cannot open offset log  \"%s\" for read\n"), "/tmp/xlog_offset_zt");
                exit(1);
        }

        //printf("index %d : %s : %d\n", index, project[index].config.server_addr, strlen(project[index].config.server_addr));
        //printf("p index %d : %s \n", index, public->server_addr);
        line_min_len          =  project[index].config.line_min_len != 0 ?  project[index].config.line_min_len  : public->line_min_len ;
        line_max_len          =  project[index].config.line_max_len != 0 ?  project[index].config.line_max_len  : public->line_max_len ;
        line_count_per        =  project[index].config.line_count_per != 0 ?  project[index].config.line_count_per  : public->line_count_per ;
        server_ip             =  strlen(project[index].config.server_addr) > 0 ?  project[index].config.server_addr  : public->server_addr ;
        server_port    =  project[index].config.server_port != 0 ?  project[index].config.server_port  : public->server_port ;
        server_retry_count    =  project[index].config.server_retry_count != 0 ?  project[index].config.server_retry_count  : public->server_retry_count ;
        server_retry_interval =  project[index].config.server_retry_interval != 0 ?  project[index].config.server_retry_interval  : public->server_retry_interval ;
        //exit(0);
        //setbuf(str, NULL);
        t_start  = get_timestamp();

        int ignore_count = 0;
        char *ignore_key[100];
        char *p;
        p = (char *) malloc (strlen(project[index].ignore) + 1);
        strcpy(p, project[index].ignore);
        while( (ignore_key[ignore_count]=strtok(p, "|+|") ) !=NULL ) {
                ignore_count++;
                p = NULL;
        }

	int w = 0;
	fpos_t pos;
	long offset_record = xFile(offset_log, NULL, 0);
	printf("Start reading log file from the offset: %ld\r\n", offset_record);
	osize = project[index].from_begin == 1 ? 0 : (( project[index].from_begin = 2 && offset_record) > 0 ? offset_record : filesize(filename));    
        printf("%d %s %s %d %d \n",index, filename, server_ip, server_port, osize);
        for (osize;;) {
		nsize = filesize(filename);
		if (nsize > osize) {
			if (!(str = fopen(filename, "r"))) {
				fprintf(stderr, _("Cannot open \"%s\" for read\n"), filename);
				exit(1);
			}
                        setvbuf(str, NULL, _IOLBF, BUFSIZ);
			if (!fseek(str, osize, SEEK_SET)){
				while(!feof(str)) {
                                        fgets(buf, line_max, str);
					//if (buf == NULL || !isdigit(buf[0]) ) {
					if (buf == NULL) {
						continue;
					}
 
				        line_count_total++;
					c_shmaddr[index].count_total = line_count_total;
					if ( strcmp(project[index].type, "nginx") == 0 && (!isdigit(buf[0]) || !is_ip_start(buf)) ) {
						continue;
					}

					int is_ignore = 0;
                                        if ( ignore_count > 0 ) {
                                                int i = 0;
                                                for(i = 0; i < ignore_count; i++) {
                                                        if (strstr(buf, ignore_key[i]) != NULL ) {
                                                                line_count_ignore++;
                                                                c_shmaddr[index].count_ignore = line_count_ignore;
                                                        	printf("%s@%s\r\n", ignore_key[i], buf);
								is_ignore = 1;
                                                                continue;
                                                        }
                                                }
                                        }

					if ( is_ignore == 1) {
						continue;
					}

					printf("#%s\r\n", buf);
					long int fgets_len = strlen(buf);
					if ( fgets_len < line_max_len && fgets_len > line_min_len ) {
                                       		sendMsg(server_ip, server_port, buf, index,  c_shmaddr);
						line_count++;
						//进入临界区
						if(!semaphore_p()) {
							exit(-8);
						}
						c_shmaddr[index].count = line_count;
						long  offset = ftell(str);
						char offsetStr[15];
						sprintf(offsetStr, "%ld", offset);
                                                printf("%s\r\n", offsetStr);
						xFile(offset_log, offsetStr, 1);
						if(!semaphore_v()) {
							exit(-9);
						}
					}
				}
			}
			//fflush(stdout);
			fclose(str);
			osize = nsize;
		}

                //异常处理:日志搜集过程中，日志文件被外部操作清空，这种情况下从头开始搜集日志
                if (nsize < osize) {
			osize = 0;
		}
		time_t t_end  = get_timestamp();
		usleep(1000000);         /* 250mS */
	}
	return 1;
}




int main(int argc, char **argv)
{
	//char **env = environ;while(*env){printf("%s\n",*env);env++;};exit(0);
	char       buf[BUFFER];
	char       fork_f_buf[BUFFER];
        char       conf_line[1024];
	FILE       *fp;
	const char *filename = "xlog.conf";
	if (argv[1]) {
		filename = argv[1];
	}
	
	syslog (LOG_NOTICE, "Xlog first daemon started.");
        char *conf_path = "/etc/xlog.conf";
	int oc; 
        int daemon = 0;
	while((oc = getopt(argc, argv, "c:dh")) != -1)
	{
		switch(oc)
		{
			case 'c':
				conf_path = optarg;
				break;
			case 'd':
				daemon = 1;
				break;
			case 'h':
				show_help();
				break;
			case '?':
				show_help();
				break;
			default:
				show_help();
		}
	}
	
	//filename = argv[1];
	int ch;
        int count = 0;
        if (!(fp = fopen(conf_path, "r"))) {
        	fprintf(stderr, _("Cannot open configure file \"%s\" for read\n"), conf_path);
        	exit(1);
    	}

      //统计日志文件数目，后面会生产相应个数的子进程，每个子进程负责分析一个日志文件
        while(!feof(fp)) {
		fgets(conf_line, 1024, fp);
                if (strstr(conf_line, "[project]") != NULL) {
                        count++;
                }
        }
        //fclose(fp); 
        get_conf(fp, &public_arr, project_arr);
        fclose(fp); 
	//printf("project_arr:%s - > %d",project_arr[0].log_path, project_index);
	//exit(0);
	
        if (!(file_log = fopen(public_arr.log_file, "a+"))) {
                fprintf(stderr, _("Cannot open xlog log  \"%s\" for read\n"), public_arr.log_file);
                exit(1);
        }

	
	if ( daemon == 1 || strcmp(public_arr.daemonize, "yes") == 0) {
        	xlog_daemon();
	}
        //signal(SIGINT, ctrlcHandle);
        signal(SIGUSR1, sig_handler);  
		
		
	int w,r;
        char fstr[count][BUFFER];
        int p_arr[count];
	int status,i,j;

         int shmid;
         key_t key_shm = ftok("/tmp/xlog_shm", (int)"q");  
         shmid= shmget(key_shm, sizeof(project_arr), IPC_CREAT);
         if(shmid== -1){                            // 申请共享内存失败
                   printf("create share memory failed : %s\n", strerror(errno));
                   exit(-1);
         }
         
        key_t key_sem = ftok("/tmp/xlog_sem", (int)'a');  
	//创建信号量
	sem_id = semget(key_sem, 1, 0666 | IPC_CREAT);

		//程序第一次被调用，初始化信号量
	if(!set_semvalue())
	{
		fprintf(stderr, "Failed to initialize semaphore\n");
		exit(EXIT_FAILURE);
	}

        if(socketpair(AF_UNIX, SOCK_STREAM, 0, proc[99].s) == -1) {
                printf("create unnamed socket pair failed");
                exit(2);
        }
        //生产子进程
	for(i=0;i<count;i++)
	{
		if(socketpair(AF_UNIX, SOCK_STREAM, 0, proc[i].s) == -1) {
			printf("create unnamed socket pair failed");
			exit(2);
		}
		//fcntl(proc[i].s[0], F_SETFL, fcntl(proc[i].s[0], F_GETFL, 0) | O_NONBLOCK);   
		//fcntl(proc[i].s[1], F_SETFL, fcntl(proc[i].s[0], F_GETFL, 0) | O_NONBLOCK);   
		//fcntl(proc[i].s[0],F_SETFD, FD_CLOEXEC);
		//fcntl(proc[i].s[1],F_SETFD, FD_CLOEXEC);
		status=fork();
		if(status==-1) {
		} else if (status == 0) {
                        //记录进程PID,后面主体部分通过PID确定当前子进程是fork的第几个子进程
			p_arr[i] = getpid();
			project_arr[i].pid = p_arr[i];
                        //很关键的地方,退出循环，避免子进程进入循环创建子进程 
			break;
		} else {
			p_arr[i] = status;
		};
	}
        //初始化进程名称、参数、环境变量等信息，为修改进程名称做准备
	init_proc_title(argc,argv);
	if(status==-1) {
		perror("fork");
		abort();  
	}else if(status==0){
		pid_t gcid = getpid();
		pid_t gpid = getppid();
		//printf("This is child process:%d,parent process:%d,count %d\n",gcid,gpid,count);
		int m;
		for(m = 0;m<count;m++) {
			//printf("p_arr[%d]:%d\n", m, p_arr[m]);
			//printf("fstr[%d] :%s\n", m, fstr[m]);
			//当前子进程名和进程列表中某个PID相等，就用当前下标作为当前子进程的位置标示
			if (p_arr[m] == gcid) {
                                //printf("我是第 %d 个子进程\n", m + 1);
				char c_title[100] = "xlog worker: ";
				//printf("fstr[%d] :%s\n", m, fstr[m]);
				strcat(c_title, project_arr[m].path);
                                //设置子进程名称
				set_proc_title(c_title);
                                //当前子进程通知它之前的进程接收自己的进程PID信息
				//#close(proc[m].s[0]);
				for(j=0;j<m;j++) {
					//#close(proc[j].s[1]);
					char pid_str[5];
					sprintf(pid_str, "%d", p_arr[m]);
					//itoa(p_arr[i], pid_str, 10);
					if ( (w = write(proc[j].s[0], pid_str, strlen(pid_str))) == -1) {
						printf("write socket error\n");
						exit(3);
					}   
                                         
				}
				char pid_str_r[10];
				memset(pid_str_r, 0, 10);
                                int get_c = 0;
                                //除了最后一个子进程有所有进程的ID信息外，其它的子进程都从后面的进程获取信息
				while(m < count - 1) {
					if ((r = read (proc[m].s[1], pid_str_r, 5)) == -1) {
						printf("read socket error\n");
						exit(4);
					} else {
						p_arr[m] = atoi(pid_str_r);
                                                get_c = get_c + 1;
						//printf("%d read pid %s\n",gcid, pid_str_r);
                                                //当前进程读取完后续子进程的PID退出循环，让程序继续后面的逻辑
                                                if (get_c >= count - m - 1) {
                                                        //printf("%d 读了 %d 个\n", gcid, get_c);
                                                    	break;
                                                }
					}
                                }
				struct conf_project *c_shmaddr;
                                c_shmaddr = shmat(shmid,NULL,0);       // 映射到子进程之中一个地址，具体由kernel 指配
                                //开始干子进程应该干的正事
				listen_log(&public_arr, project_arr, m, c_shmaddr);
				//listen_log(&public_arr, c_shmaddr, m);
				break;
			}
		}
		//exit(0);
	}else{
		char f_title[1000] = "xlog master ";
                strcat(f_title, conf_path);
		set_proc_title(f_title);
		//printf("This is parent process%d\n",getpid());
		int status;
		pid_t pid,pr;
		/*
		pid=wait(&status);
		if(status!=0)
			printf("Child Process with PID %ld aborted with  status   %d\n", (long)pid, status, get_timestamp());
                */

                //printf("fork_f_buf_start\n");
                //while(1) {
                //        if ((r = read (proc[99].s[1], fork_f_buf, BUFFER)) == -1) {
                //                printf("read socket error\n");
                //                exit(4);
                //        } else {
                //                printf("fork_f_buf%s\n", fork_f_buf);

                //        }
                //        sleep(1);
                //}
		pr = wait(&status);
		if(WIFEXITED(status)){
			printf("\nthe child process %d exit normally\n", pr);
			printf("\nthe return code is %d\n",WEXITSTATUS(status));
		}else{
			printf("\nthe child process %d exit abnormally\n", pr);
		}
		del_semvalue();
	}
}

