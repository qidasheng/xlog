#include "xlog.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#define BUFFER_SIZE 40960

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


int addLog(char *msg) {
	const char *logpath = "xlog.log";
	FILE *fp;
        if (!(fp = fopen(logpath, "ab+"))) {
            fprintf(stderr, _("Cannot open log file \"%s\" for read\n"), logpath);
            exit(1);
        }
	if (msg != NULL) {
		fwrite(msg, sizeof(msg), strlen(msg), fp);	
	}
	fclose(fp);
	return 0;
}

int createConn(char *ip, unsigned int port) {
    int client_socket = -1;
    //设置一个socket地址结构client_addr,代表客户机internet地址, 端口
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof (client_addr)); //把一段内存区的内容全部设置为0
    client_addr.sin_family = AF_INET; //internet协议族
    client_addr.sin_addr.s_addr = htons(INADDR_ANY); //INADDR_ANY表示自动获取本机地址
    client_addr.sin_port = htons(0); //0表示让系统自动分配一个空闲端口
    //创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        printf("Create Socket Failed!\n");
        return -1;
    }
    //把客户机的socket和客户机的socket地址结构联系起来
    if (bind(client_socket, (struct sockaddr*) &client_addr, sizeof (client_addr))) {
        printf("Client Bind Port Failed!\n");
	return -2; 
   }

    //make_socket_non_blocking(client_socket);

    //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof (server_addr));
    server_addr.sin_family = AF_INET;

    if (inet_aton(ip, &server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
    {
        printf("Server IP Address Error!\n");
        return -3;
    }
    server_addr.sin_port = htons(port);
    socklen_t server_addr_length = sizeof (server_addr);
    //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
    if ( connect(client_socket, (struct sockaddr*) &server_addr, server_addr_length) < 0 ) 
    {
    	return -4;
    }
    struct timeval timeout = {30,0}; 
    //设置发送超时
    setsockopt(client_socket, SOL_SOCKET,SO_SNDTIMEO, (char *)&timeout,sizeof(struct timeval));
    //设置接收超时
    setsockopt(client_socket, SOL_SOCKET,SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));
    return client_socket;
    /*
    execvp(arg[0], arg);
    perror("execvp");
     */
}


void sendMsg(char *ip, unsigned int port, char *msg, int index, conf_project *c_shmaddr){
    int retry_count = 0;
    while (client_socket < 0 && server_retry_count > retry_count ) {
	client_socket = createConn(ip, port);
	retry_count++;
	sleep(server_retry_interval);
    }

    if (client_socket < 0) {
        printf("Can Not Connect To IP:%s,PORT:%d\n", ip, port);
	exit(0);
    }
    int i;
    int sendLen = 0;
    int length = 0;
    char buffer_send[BUFFER_SIZE];
    char buffer_recv[BUFFER_SIZE];
    bzero(buffer_send, BUFFER_SIZE);
    bzero(buffer_recv, BUFFER_SIZE);
    //strcpy(buffer, "输入内容为：");
    strcat(buffer_send, msg);
    //strcat(buffer, "\n");
    //printf("Input data :%s", buffer_send);
    //向服务器发送buffer中的数据
    sendLen = send(client_socket, buffer_send, strlen(buffer_send), 0);
    if (sendLen < 0) {
  	printf("消息'%s'发送给%s失败！错误代码是%d，错误信息是'%s'\n", buffer_send, ip,  errno, strerror(errno));
    }
   
    if (sendLen = 0) {
    	client_socket = -1;
    }

    //从服务器接收数据到buffer中
    length = recv(client_socket, buffer_recv, BUFFER_SIZE, 0);

    if (length <= 0) {
        printf("Recieve Data From Server %s Failed!\n", ip);
        client_socket = -1;
        //exit(1);
    }

    if ( strcmp(buffer_recv, "OK\n")==0 ) {
	line_count_ok++;
	c_shmaddr[index].count_ok = line_count_ok;
	//printf("From Server %s :\t%s\n", ip, buffer_recv);
    }
    //printf("From Server %s :\t%s\n", ip, buffer_recv);
    //printf("%s", buffer_recv);
    //close(client_socket);client_socket = -1;
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
	FILE       *str;
	char *filename;
	filename = project[index].path;
       
	if (!(str = fopen(filename, "r"))) {
		fprintf(stderr, _("Cannot open web log  \"%s\" for read\n"), filename);
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
	osize = project[index].from_begin != 0 ? 0 : filesize(filename);    
        printf("%d %s %s %d %d \n",index, filename, server_ip, server_port, osize);
        //exit(0);
        //setbuf(str, NULL);
        t_start  = get_timestamp();
	int w = 0;
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

					if ( strlen(project[index].ignore) > 5  &&  strstr(buf, project[index].ignore) != NULL ) {
						line_count_ignore++;
						c_shmaddr[index].count_ignore = line_count_ignore;
						continue;
					}
					//printf("%s", buf);
					long int fgets_len = strlen(buf);
					if ( fgets_len < line_max_len && fgets_len > line_min_len ) {
                                       		sendMsg(server_ip, server_port, buf, index,  c_shmaddr);
						line_count++;
						//进入临界区
						if(!semaphore_p()) {
							exit(-8);
						}
						c_shmaddr[index].count = line_count;
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

