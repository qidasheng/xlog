#include "parse_conf.h"

char *get_hostname () {
    gethostname(hostname, 1024);  
    return hostname;
}

char *get_ip (const char *eth_name) {
        int fd;
        struct ifreq ifr;

        fd = socket(AF_INET, SOCK_DGRAM, 0);

        /* I want to get an IPv4 IP address */
        ifr.ifr_addr.sa_family = AF_INET;

        /* I want IP address attached to "eth0" */
        strncpy(ifr.ifr_name, trim_str(eth_name), IFNAMSIZ-1);

        ioctl(fd, SIOCGIFADDR, &ifr);

        close(fd);

        /* display result */
        //sprintf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
        sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
        return ip;
}

int position( const char *s, int c ) {
	if ( s == NULL ) {
		return -1;
	}
	int pos;
	char *p;
	p = index(s, c);
	return p == NULL ? -1 : p - s;
}


void xstrcpy (char *p, const char *buf) {
        strncpy(p, buf,LINE - 1);
}


static struct conf_project * init_conf_project (void)	{
	struct conf_project *project;
	if ((project = malloc (sizeof (*project))) == NULL) {
	     return NULL;
	}
	project->pid = 0;
	project->count = 0;
	project->count_ok = 0;
	project->count_total = 0;
	project->count_ignore = 0;
	
	return project;
}




int parse_project(char *line, conf_public *public_arr,  conf_project *project_arr, int index) {
	//line[strlen(line)-1]='\0';
	//printf("--->%s<---", line);
        if ( strlen(line) < 3 || line == NULL || line == "\r\n" ||  line[0] == '#' ) {
		return 0;
        }
        char *key, *val;
        int  i=0;
        char *p[2];
        char *buf = line;
        char *key_name = NULL;
        char *val_str = NULL;
        while((p[i]=strtok(buf,"="))!=NULL) {
                i++;
                buf=NULL;
        }
        if (p[0] != NULL && p[1] != NULL) {
                key = trim_str(p[0]);
		if ((key_name = "add_prefix" , strcmp(key, key_name))!=0) {
                	val = trim_str(p[1]);
		} else {
                	val = p[1];
			if (val[strlen(val) - 1] == '\n') {
                		val[strlen(val) - 1] = '\0';
        		}
			if (val[0] == '@') {
				val[0] = ' ';
				val = get_ip(val);
			} else if (strcmp(val, "date")==0) {
				val = get_date();	
			} else if (strcmp(val, "hostname")==0) {
				val = get_hostname();	
			}
		}
		//printf("%s--->%s\r\n", key, val);
                if (index != -1) {
                        if ((key_name = "name" , strcmp(key, key_name))==0) {                           
                                xstrcpy(project_arr[index].name,  val);
                        } else if ((key_name = "from_begin" , strcmp(key, key_name))==0) {                          
                                project_arr[index].from_begin = atoi(val);
                        } else if ((key_name = "path" , strcmp(key, key_name)) == 0) {                          
                                xstrcpy(project_arr[index].path , val);
                        } else if ((key_name = "type" , strcmp(key, key_name))==0) {                         
                                xstrcpy(project_arr[index].type ,val);						
                        } else if ((key_name = "ignore" , strcmp(key, key_name))==0) {                         
                                xstrcpy(project_arr[index].ignore ,val);						
                        } else {
				if ((key_name = "line_min_len" , strcmp(key, key_name))==0) {
						project_arr[index].config.line_min_len = atoi(val);
				} else if ((key_name = "line_max_len" , strcmp(key, key_name)) == 0) {
						project_arr[index].config.line_max_len = atoi(val);
				} else if ((key_name = "line_count_per" , strcmp(key, key_name))==0) {
						project_arr[index].config.line_count_per = atoi(val);
				} else if ((key_name = "server_addr" , strcmp(key, key_name))==0) {
						xstrcpy(project_arr[index].config.server_addr, val);
				} else if ((key_name = "server_port" , strcmp(key, key_name))==0) {
						project_arr[index].config.server_port = atoi(val);
				} else if ((key_name = "server_retry_count" , strcmp(key, key_name))==0) {
						project_arr[index].config.server_retry_count = atoi(val);
				} else if ((key_name = "server_retry_interval" , strcmp(key, key_name))==0) {
						project_arr[index].config.server_retry_interval = atoi(val);								
				} else if ((key_name = "add_prefix" , strcmp(key, key_name))==0) {
						xstrcpy(project_arr[index].config.add_prefix, val);
				}
                
		       }
		} else {
                        if ((key_name = "line_min_len" , strcmp(key, key_name))==0) {
                                public_arr->line_min_len = atoi(val);
                        } else if ((key_name = "line_max_len" , strcmp(key, key_name)) == 0) {
                                public_arr->line_max_len = atoi(val);
                        } else if ((key_name = "line_count_per" , strcmp(key, key_name))==0) {
                                public_arr->line_count_per = atoi(val);
                        } else if ((key_name = "server_addr" , strcmp(key, key_name))==0) {
                                xstrcpy(public_arr->server_addr, val);
                        } else if ((key_name = "server_port" , strcmp(key, key_name))==0) {
                                public_arr->server_port = atoi(val);
                        } else if ((key_name = "server_retry_count" , strcmp(key, key_name))==0) {
                                public_arr->server_retry_count = atoi(val);
                        } else if ((key_name = "server_retry_interval" , strcmp(key, key_name))==0) {
                                public_arr->server_retry_interval = atoi(val);								
                        } else if ((key_name = "log_file" , strcmp(key, key_name))==0) {
                                xstrcpy(public_arr->log_file, val);
                        } else if ((key_name = "log_level" , strcmp(key, key_name))==0) {
                                xstrcpy(public_arr->log_level, val);
                        } else if ((key_name = "listen_addr" , strcmp(key, key_name))==0) {
                                xstrcpy(public_arr->listen_addr, val);
                        } else if ((key_name = "listen_port" , strcmp(key, key_name))==0) {
                                public_arr->listen_port = atoi(val);	
                        } else if ((key_name = "add_prefix" , strcmp(key, key_name))==0) {
                                xstrcpy(public_arr->add_prefix, val);
                        }
                }
        }
	return 0;

}



int get_conf(FILE *f, conf_public *public_arr, conf_project *project_arr) {    
    	char buf[BUFFER];
    	char line[LINE];
	int  n = 0;

	int is_project_conf = 0;
	int project_conf_index = 0;
	int project_index = 0;
	fseek(f, 0, SEEK_SET);
	while(!feof(f)) {
		fgets(line, LINE, f);
		//printf("%c -> ", *line);
        	if ( *line != '#' && strstr(line, "[project]") != NULL ) { 
			is_project_conf = 1;
			project_conf_index = 0;
			project_index++;
		} else {
			if (is_project_conf) {
				project_conf_index++;
			} else {
				is_project_conf = 0;
			}
		}
			
		//printf("%d:%d:%s -> ", project_index, project_conf_index, line);
		if (is_project_conf && project_conf_index > 0) {
			parse_project(line, NULL, project_arr, project_index - 1);
		} else {
			parse_project(line, public_arr, NULL, -1);
		}
		n++;
	}
	return 0;
}


