//#include <stdlib.h>
//#include <stdio.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <malloc.h>
//#include <error.h>
//#include <errno.h>
#include <string.h>
//#include <ctype.h>
#include "xlog.h"
void init_proc_title(int argc, char **argv) {
        int i;
        //printf("参数个数：%d\n",argc);
        arg_start = argv[0];
        arg_end = argv[argc-1] + strlen(argv[argc-1])+1;
        env_start = environ[0];
        //转移argv参数
        for(i=0; i<argc; i++)
                argv[i] = strdup(argv[i]);
}
void set_proc_title(const char *title) {
        int tlen = strlen(title)+1;
        int i;
        char *p;
        //argv本身空间大小不足以放下title，所以需要挪用紧跟其后environ的空间
        if(arg_end-arg_start < tlen && env_start==arg_end) {
                char *env_end = env_start;
                for(i=0; environ[i]; i++) {
                        if(env_end == environ[i]) {
                                env_end = environ[i] + strlen(environ[i]) + 1;
                                environ[i] = strdup(environ[i]);
                        } else {
                                break;
                        }
                }
                arg_end = env_end;
                env_start = NULL;
        }
        i = arg_end - arg_start;
        //用新标题填充原标题位置，达到改写进程的名称的目的
        if(tlen==i) {
                strcpy(arg_start, title);
        } else if(tlen < i) {
                strcpy(arg_start, title);
                //memset(arg_start+tlen, 0, i-tlen);
                //memset(arg_start,0,i);
                //strcpy(arg_start + (i - tlen),title);
        } else {
                *(char *)memcpy(arg_start, title, i-1) = '\0';
        }
        if(env_start) {
                p = strchr(arg_start, ' ');
                if(p) *p = '\0';
        }
}


