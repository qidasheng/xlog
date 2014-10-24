#include "xlog.h"



int  line_min_len;
int  line_max_len;
int  line_count_per;
char *server_ip;
long server_port;
int  server_retry_count;
int  server_retry_interval;

char log_file[LINE];
char log_level[LINE];


char *trim_str (char *str);

void xstrcpy (char *p, const char *buf) {
        strncpy(p, buf, LINE - 1);
}


static struct conf_project * init_conf_project (void)
{
   struct conf_project *project;

   if ((project = malloc (sizeof (*project))) == NULL)
      return NULL;
   project->pid = 0;
   project->count = 0;
   project->count_ok = 0;
   project->count_total = 0;
   project->count_ignore = 0;

   return project;
}




int parse_project(char *line, conf_public *public_arr,  conf_project *project_arr, int index) 
{
        if (line == NULL || line == "\r\n") {
                return 0;
        }
        const char *key, *val;
        int  i=0;
        char *p[2];
        char *buf = line;
        char *key_name = NULL;
        while((p[i]=strtok(buf,"="))!=NULL) {
                i++;
                buf=NULL;
        }
        if (p[0] != NULL && p[1] != NULL) {
                key = trim_str(p[0]);
                val = trim_str(p[1]);
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
                        }
                }
        }
	return 0;

}



int get_conf(FILE *f, conf_public *public_arr, conf_project *project_arr) {    
    char       buf[BUFFER];
    char       conf_line[1024];
	int n = 0;

    int is_project_conf = 0;
    int project_conf_index = 0;
	int project_index = 0;
	fseek(f, 0, SEEK_SET);
	while(!feof(f)) {
		fgets(conf_line, 1024, f);
        if (strstr(conf_line, "[project]") != NULL) { 
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
		
		if (is_project_conf && project_conf_index > 0) {
			parse_project(conf_line, NULL, project_arr, project_index - 1);
		} else {
			parse_project(conf_line, public_arr, NULL, -1);
		}
		n++;
	}
	return 0;
}

