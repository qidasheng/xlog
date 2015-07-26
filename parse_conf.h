#include "xlog.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define BUFFER  4096
int  line_min_len;
int  line_max_len;
int  line_count_per;
char *server_ip;
long server_port;
int  server_retry_count;
int  server_retry_interval;
char *add_prefix;

char log_file[LINE];
char log_level[LINE];


char *trim_str (char *str);
char *get_date();
char ip[16];
char hostname[1024];

