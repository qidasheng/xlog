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

