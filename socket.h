#include "xlog.h"
extern char *server_ip;
extern long server_port;
extern int  server_retry_count;
extern int  server_retry_interval;
int client_socket = -1;
long line_count_ok = 0;

