#include "general.h"
int send_404status(SSL *ssl, int socketfd) {
    char path[128] = PATH "/404.html";
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        SSL_send(ssl, socketfd, "404", 3);
        return 0;
    }

    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char head_buff[512] = "HTTP/1.1 404 Not Found\r\n";
    strcat(head_buff, "Server: SugarCake\r\n");
    sprintf(head_buff + strlen(head_buff), "Content-Length: %d\r\n", size);
    strcat(head_buff, "\r\n");
    SSL_send(ssl, socketfd, head_buff, strlen(head_buff));

    char data[1024] = {};
    int num = 0;
    while ((num = read(fd, data, 1024)) > 0) {
        SSL_send(ssl, socketfd, data, num);
    }

    close(fd);
    return 0;
}

int send_file(SSL *ssl, int socketfd, char *filename, char * range) {
    int *r = get_range(range);
    printf("range1:\n %d\n", r[0]);
    printf("range2:\n %d\n", r[1]);

    if (filename == NULL || socketfd < 0) {
        SSL_send(ssl, socketfd, "err", 3);
        return -1;
    }
    if (strcmp(filename, "/") == 0) {
        filename = "/index.html";
    }
    char path[128] = PATH;
    strcat(path, filename);
    printf("\n\n[%s]\n\n", path);
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        send_404status(ssl, socketfd);
        return -1;
    }

    int offset = 0;
    int size = lseek(fd, 0, SEEK_END);
    if(r[0] != -1){
        offset = r[0];
        size = size - offset;
    }
    if(r[1] != -1){
        size = r[1] - offset + 1;
    }
    lseek(fd, offset, SEEK_SET);
    char head_buff[512] = "HTTP/1.1 ";
    if(r[0] == -1 && r[1] == -1)
        strcat(head_buff, "200 OK\r\n");
    else
        strcat(head_buff, "206 Partial Content\r\n");
    sprintf(head_buff + strlen(head_buff), "Content-Length: %d\r\n", size);
    strcat(head_buff, "\r\n");
    SSL_send(ssl, socketfd, head_buff, strlen(head_buff));
    printf("send file:\n%s\n", head_buff);
    char data[1024] = {};  
    while(size > 0){
      if(size >= 1024){
          read(fd, data, 1024);
          SSL_send(ssl, socketfd, data, 1024);
          size = size - 1024;
      }
      else{
          read(fd, data, size);
          SSL_send(ssl, socketfd, data, size);
          size = 0;
      }
    }

    close(fd);
    return 0;
}
