#include "xlog.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


int  child_num = 0;
int  thread_socket = -1;
FILE *file_log;

struct process {
        int s[2];
};

int p_arr[100];
struct process proc[100];

extern int  line_min_len;
extern int  line_max_len;
extern int  line_count_per;
extern char *server_ip;
extern long server_port;
extern int  server_retry_count;
extern int  server_retry_interval;
extern char *add_prefix;
long line_count = 0;
long line_count_total = 0;
long line_count_ignore = 0;
char *filename;
long  offset = 0;
FILE *xlogFp,*offset_log;
char dynamic_path[LINE];
char *new_path;

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
        .daemonize = "",
        .add_prefix  = ""
};

time_t t_start  = 0;

extern void send_msg(char *ip, unsigned int port, char *msg, int index, conf_project *c_shmaddr);
char *get_date();

char *get_dynamic_path(path_format) {
    //if (!strchr(path_format, "%")) {
    //    return path_format;
    //}
    time_t t = time(0);
    strftime(dynamic_path, sizeof(dynamic_path), path_format, localtime(&t));
    return dynamic_path;
}

char *ltoa(long N, char *str, int base)
{
      register int i = 2;
      long uarg;
      int bufsize = sizeof(long) * 8 + 1;
      char *tail, *head = str, buf[bufsize];

      if (36 < base || 2 > base)
            base = 10;                    /* can only use 0-9, A-Z        */
      tail = &buf[bufsize - 1];           /* last character position      */
      *tail-- = '\0';

      if (10 == base && N < 0L)
      {
            *head++ = '-';
            uarg    = -N;
      }
      else  uarg = N;

      if (uarg)
      {
            for (i = 1; uarg; ++i)
            {
                  register ldiv_t r;

                  r       = ldiv(uarg, base);
                  *tail-- = (char)(r.rem + ((9L < r.rem) ?
                                  ('A' - 10L) : '0'));
                  uarg    = r.quot;
            }
      }
      else  *tail-- = '0';

      memcpy(head, ++tail, i);
      return str;
}


long  xFile(FILE *fp, char *msg, int mode) {
	long  log_offset = 0;
	char line[1024]; 
	if (msg != NULL) {
		char *log_info;
		char *date;
		long info_len;
		if ( mode == 1) {
			info_len = strlen(msg)  + strlen("\r\n") + 1;
		} else {
			date = get_date();
			info_len = strlen(date) + strlen("	") + strlen(msg)  + strlen("\r\n") + 1;
		}
		log_info = (char *) malloc(info_len);
		if (log_info != NULL) { 
		        if ( mode == 1) {
				fseek(fp, 0, SEEK_SET);
				sprintf(log_info, "%s\r\n", msg);
			} else {
				fseek(fp, 0, SEEK_END);
				sprintf(log_info, "%s,%s\r\n", date, msg);
			}
			fwrite(log_info, info_len, 1, fp);
			free(log_info);	
		}
	} else {
		fseek(fp, 0, SEEK_SET);
                fgets(line, 1024, fp);
		log_offset = atol(line);
		return log_offset;
	}
	return 0;
}


void addLog (char *msg) {
	xFile(file_log, msg, 2);
}

void signal_process (int sig) {
        if(sig != SIGALRM){
             exit(10);
        }
        long  offset = ftell(xlogFp);
        char offsetStr[50];
	ltoa(offset, offsetStr, 10);
        if ( offset != -1 ) {
                xFile(offset_log, offsetStr, 1);
        }

        if (xlogFp) {
                signal(SIGALRM, signal_process);
                alarm(1);
        }
}



void sig_handler( int sig) {
       if( sig == SIGUSR1 ) {
	    get_conf();
       }
}

char *get_date() {
    time_t t = time(0);
    static char s[32];
    strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return s;
}

char * xl_str_contact(char *first, char *second) {
     char *result;
     result = (char*) malloc(strlen(first) + strlen(second) + 1);
     if( result == NULL ){
	debug("Error: malloc failed");
        exit(EXIT_FAILURE);
     }
     strcpy(result, first);
     strcat(result, second);
     return result;
}

unsigned long long get_datetime() {
    unsigned long long datetime = 0;
    char *datetimestr;
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    sprintf(datetimestr, "%d%d%d%d%d%d", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
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



void error_handler (const char *func, char *file, int line, char *msg) {
   fprintf (stderr, "\nxlog - version %s - %s %s\n", "0.1",  __DATE__, __TIME__);
   fprintf (stderr, "\nAn error has occurred");
   fprintf (stderr, "\nError occured at: %s - %s - %d", file, func, line);
   fprintf (stderr, "\nMessage: %s\n\n", msg);
   exit (EXIT_FAILURE);
}





char *rtrim_str (char *str) {
   char *p;
   if (!str)
      return NULL;
   if (!*str)
      return str;
   for (p = str + strlen (str) - 1; (p >= str) && isspace (*p); --p);
   p[1] = '\0';
   return str;
}

char *ltrim_str (char *str) {
   char *p;
   if (!str)
      return NULL;
   if (!*str)
      return str;
   for (p = str ; (p <= str) && isspace (*p); ++p);
   str = p;
   return str;

}


char *trim_str (char *str) {
   return ltrim_str(rtrim_str(str));
}



static size_t filesize(const char *filename) {
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
        int ip_min_len = 7;
        int ip_max_len = 15;
        int dot_count = 0;
        int step  = 1;
        int len = 0;
        while (*p != '\0' || len <= ip_max_len) {
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
                p++;
                len++;

        }
	free(tmp);
	tmp = NULL;
        if (dot_count != 3) {
                return 0;
        }
        return 1;
}

int file_exists(char *filename) {
	return (access(filename, 0) == 0);
}

static void xlog_daemon() {
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

void kill_all_child(int sig) {
	int i = 0;
	for( i=0; i < child_num; i++) {
		kill(p_arr[i], SIGQUIT);
	}
	exit(0);
}

void fault_tolerant () {
        alarm(0);
        usleep(1250000);
        fclose(xlogFp);
        if (!(xlogFp = fopen(filename, "r"))) {
                debug("Cannot open web log  \"%s\" for read", filename);
                exit(1);
        }
        signal(SIGALRM, signal_process);
        alarm(1);
        usleep(1250000);
}


int listen_log(conf_public *public,conf_project *project, int index, conf_project *c_shmaddr) {
	char       buffer[BUFFER];
	int        line_max = 32 * 1024;
	int 	   line_count = 0;
	int 	   valid_count = 0;
	char       buf[line_max];
	char       msg[line_max];
	char       fork_buf[line_max];
	size_t     osize, nsize;
	char 	   *tmp_path;
	//filename = project[index].path;
	tmp_path = get_dynamic_path(project[index].path);
	filename = (char *) malloc(strlen(tmp_path));
        strncpy(filename, tmp_path, strlen(tmp_path)+1);
	if (!(xlogFp = fopen(filename, "r"))) {
		debug("Cannot open web log  \"%s\" for read", filename);
		exit(1);
	}

	char *offset_log_path = xl_str_contact("/tmp/xlog_offset_", project[index].name);

	if( !file_exists(offset_log_path) )   {
		FILE *fc = fopen(offset_log_path, "w+");
		fclose(fc);
	}   

	if (!( offset_log = fopen(offset_log_path, "r+") )) {
                debug("Cannot open offset record file  \"%s\"", offset_log_path);
                exit(1);        
        } 

        line_min_len          =  project[index].config.line_min_len != 0 ?  project[index].config.line_min_len  : public->line_min_len ;
        line_max_len          =  project[index].config.line_max_len != 0 ?  project[index].config.line_max_len  : public->line_max_len ;
        line_count_per        =  project[index].config.line_count_per != 0 ?  project[index].config.line_count_per  : public->line_count_per ;
        server_ip             =  strlen(project[index].config.server_addr) > 0 ?  project[index].config.server_addr  : public->server_addr ;
        server_port    	      =  project[index].config.server_port != 0 ?  project[index].config.server_port  : public->server_port ;
        server_retry_count    =  project[index].config.server_retry_count != 0 ?  project[index].config.server_retry_count  : public->server_retry_count ;
        server_retry_interval =  project[index].config.server_retry_interval != 0 ?  project[index].config.server_retry_interval  : public->server_retry_interval ;
	add_prefix            =  strlen(project[index].config.add_prefix) > 0 ?  project[index].config.add_prefix  : public->add_prefix ;
	if (add_prefix[strlen(add_prefix) - 1] == '\n') {
		add_prefix[strlen(add_prefix) - 1] = '\0';
	}
        signal(SIGPIPE, SIG_IGN);
        signal (SIGINT,  signal_process);
        signal (SIGKILL, signal_process);
        signal (SIGQUIT, signal_process);
        signal (SIGTERM, signal_process);
        signal (SIGHUP,  signal_process);

        t_start  = get_timestamp();

        int ignore_count = 0;
        char *ignore_key[100];
        char *p;
        char *multi_line_log ;
	multi_line_log = (char *) malloc(line_max * (line_count_per + 1));
	strcpy(multi_line_log, "");
	
        p = (char *) malloc (strlen(project[index].ignore) + 1);
        strcpy(p, project[index].ignore);
        while( (ignore_key[ignore_count]=strtok(p, "|+|") ) !=NULL ) {
                ignore_count++;
                p = NULL;
        }

	int w = 0;
	fpos_t pos;
	long offset_record = xFile(offset_log, NULL, 0);
	debug("%s offset record: %ld", filename,  offset_record);
	osize = project[index].from_begin == 1 ? filesize(filename) : (project[index].from_begin = 2 && offset_record > 50 ? offset_record : 0);    
        debug("Start info:%d %s %s %d %d",index, filename, server_ip, server_port, osize);
        signal(SIGALRM, signal_process);
        alarm(1);
	int eq_count = 0;
        for (osize;;) {
		nsize = filesize(filename);
		if (nsize > osize) {
			if (!fseek(xlogFp, osize, SEEK_SET)) {
				while( fgets(buf, line_max, xlogFp) != NULL ) {
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
								is_ignore = 1;
                                                                continue;
                                                        }
                                                }
                                        }

					if ( is_ignore == 1) {
						continue;
					}

					long int fgets_len = strlen(buf);
					if ( fgets_len < line_max_len && fgets_len > line_min_len ) {
						if (strlen(add_prefix) > 1) {
							sprintf(msg, "%s    %s", add_prefix, buf);
						}
						strcat(multi_line_log , msg);
						line_count++;
						valid_count++;
						if ( line_count >= line_count_per ) {
                                       			send_msg(server_ip, server_port, multi_line_log, index,  c_shmaddr);
							line_count = 0;
							strcpy(multi_line_log, "");
						}
						c_shmaddr[index].count = valid_count;
					}
				}
			}
			osize = nsize;
			
		}

		//容错
                if (nsize < osize) {
			fault_tolerant();
			osize = 0;
		}
		
		//动态路径切换
                if (nsize == osize && eq_count++ && eq_count > 10) {
			eq_count = 0;
			tmp_path = get_dynamic_path(project[index].path);
			new_path = (char *) malloc(strlen(tmp_path));
        		strncpy(new_path, tmp_path, strlen(tmp_path)+1);
			//新生产的路径如果存在就开始从新的路径读取信息
			if (strcmp(filename, new_path) !=0) {
			        if(file_exists(new_path))   {
					filename = new_path;
					fault_tolerant();
					osize = 0;
        			}	 
			}
		}
		time_t t_end  = get_timestamp();
		usleep(1000000);  
	}
	return 1;
}




int main(int argc, char **argv)
{
	char       buf[BUFFER];
	char       fork_f_buf[BUFFER];
        char       conf_line[BUFFER];
	FILE       *fp;
	const char *filename = "xlog.conf";
	if (argv[1]) {
		filename = argv[1];
	}
	
	syslog (LOG_NOTICE, "Xlog first daemon started.");
        char *conf_path = "/etc/xlog/xlog.conf";
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
	
	int ch;
        int count = 0;
        if (!(fp = fopen(conf_path, "r"))) {
        	printf("Cannot open configure file \"%s\" for read\n", conf_path);
        	exit(1);
    	}

        while( fgets(conf_line, BUFFER, fp) != NULL ) {
		//printf("line:%c\n", conf_line[0]);
                if ( conf_line[0] == '#' ) {
                        continue;
                }
                if (strstr(conf_line, "[project]") != NULL) {
                        count++;
                }
        }
	child_num = count;
        get_conf(fp, &public_arr, project_arr);
	fclose(fp);
        if( !file_exists(public_arr.log_file) )   {
                printf("Create xlog log  \"%s\"", public_arr.log_file);
                FILE *fc = fopen(public_arr.log_file, "w+");
                fclose(fc);
        }
	
        if (!(file_log = fopen(public_arr.log_file, "a+"))) {
                printf("Cannot open xlog log  \"%s\" for read\n", public_arr.log_file);
                exit(1);
        }
	setvbuf(file_log, NULL, _IOLBF, BUFSIZ);
	if ( daemon == 1 || strcmp(public_arr.daemonize, "yes") == 0) {
        	xlog_daemon();
	}

        signal(SIGUSR1, sig_handler);  
		
		
	int w,r;
        char fstr[count][BUFFER];
        int p_arr[count];
	int status,i,j;

        int shmid;
        key_t key_shm = ftok("/tmp", 147);  
        shmid= shmget(key_shm, sizeof(project_arr), IPC_CREAT);
        if(shmid== -1){                      
                  debug("create share memory failed : %s", strerror(errno));
                  exit(-1);
        }
         
        key_t key_sem = ftok("/var", 147);  
	sem_id = semget(key_sem, 1, 0666 | IPC_CREAT);

	if(!set_semvalue()){
		debug("Failed to initialize semaphore");
		exit(EXIT_FAILURE);
	}

        if(socketpair(AF_UNIX, SOCK_STREAM, 0, proc[99].s) == -1) {
                debug("create unnamed socketpair failed");
                exit(2);
        }
	for(i=0;i<count;i++)
	{
		if(socketpair(AF_UNIX, SOCK_STREAM, 0, proc[i].s) == -1) {
			debug("create unnamed socket pair failed");
			exit(2);
		}
		status=fork();
		if(status==-1) {
		} else if (status == 0) {
			p_arr[i] = getpid();
			project_arr[i].pid = p_arr[i];
			break;
		} else {
			p_arr[i] = status;
		};
	}
	init_proc_title(argc,argv);
	if(status==-1) {
		perror("fork");
		abort();  
	} else if (status==0){
		pid_t gcid = getpid();
		pid_t gpid = getppid();
		int m;
		for(m = 0;m<count;m++) {
			if (p_arr[m] == gcid) {
				char c_title[100] = "xlog worker: ";
				strcat(c_title, project_arr[m].path);
				set_proc_title(c_title);
				for(j=0;j<m;j++) {
					char pid_str[5];
					sprintf(pid_str, "%d", p_arr[m]);
					if ( (w = write(proc[j].s[0], pid_str, strlen(pid_str))) == -1) {
						debug("write socketpair error");
						exit(3);
					}   
                                         
				}
				char pid_str_r[10];
				memset(pid_str_r, 0, 10);
                                int get_c = 0;
				while(m < count - 1) {
					if ((r = read (proc[m].s[1], pid_str_r, 5)) == -1) {
						debug("read socketpair error");
						exit(4);
					} else {
						p_arr[m] = atoi(pid_str_r);
                                                get_c = get_c + 1;
                                                if (get_c >= count - m - 1) {
                                                    	break;
                                                }
					}
                                }
				struct conf_project *c_shmaddr;
                                c_shmaddr = shmat(shmid,NULL,0);     
				listen_log(&public_arr, project_arr, m, c_shmaddr);
				break;
			}
		}
	}else{
		char f_title[1000] = "xlog master: ";
                strcat(f_title, conf_path);
		set_proc_title(f_title);
		int status;
		pid_t pid,pr;
        	signal(SIGPIPE, SIG_IGN);
        	signal (SIGINT,  kill_all_child);
        	signal (SIGKILL, kill_all_child);
        	signal (SIGQUIT, kill_all_child);
        	signal (SIGTERM, kill_all_child);
        	signal (SIGHUP,  kill_all_child);
		pr = wait(&status);
		if(WIFEXITED(status)){
			debug("the child process %d exit normally", pr);
			debug("the return code is %d",WEXITSTATUS(status));
		}else{
			debug("the child process %d exit abnormally", pr);
		}
		del_semvalue();
	}
}

