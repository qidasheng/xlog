#include "socket.h"
int createConn(char *ip, unsigned int port) {
    int client_socket = -1;
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof (client_addr)); 
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        debug("Create Socket Failed!\n");
        return -1;
    }
    if (bind(client_socket, (struct sockaddr*) &client_addr, sizeof (client_addr))) {
        debug("Client Bind Port Failed!\n");
        return -2;
    }
    //make_socket_non_blocking(client_socket);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_aton(ip, &server_addr.sin_addr) == 0) {
        debug("Server IP Address Error!\n");
        return -3;
    }
    server_addr.sin_port = htons(port);
    socklen_t server_addr_length = sizeof (server_addr);
    if ( connect(client_socket, (struct sockaddr*) &server_addr, server_addr_length) < 0 ) {
        return -4;
    }
    struct timeval timeout = {30,0};
    setsockopt(client_socket, SOL_SOCKET,SO_SNDTIMEO, (char *)&timeout,sizeof(struct timeval));
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
        debug("Can Not Connect To IP:%s,PORT:%d\n", ip, port);
        exit(0);
    }

    int i;
    int sendLen = 0;
    int length = 0;
    char buffer_send[BUFFER_SIZE];
    char buffer_recv[BUFFER_SIZE];
    bzero(buffer_send, BUFFER_SIZE);
    bzero(buffer_recv, BUFFER_SIZE);
    strcat(buffer_send, msg);
    sendLen = send(client_socket, buffer_send, strlen(buffer_send), 0);
    if ( errno !=EINTR ) {
        client_socket = -1;
        debug("消息'%s'发送给%s失败！错误代码是%d，错误信息是'%s'\n", buffer_send, ip,  errno, strerror(errno));
    }

    //从服务器接收数据到buffer中
    length = recv(client_socket, buffer_recv, BUFFER_SIZE, 0);

    if ( errno !=EINTR ) {
        client_socket = -1;
        debug("%d recv %s errno %d %s\n", length, buffer_recv, errno, strerror(errno));
    } else {
        line_count_ok++;
        c_shmaddr[index].count_ok = line_count_ok;


    }
    //close(client_socket);client_socket = -1;
}



