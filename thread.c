#include "general.h"

static inline void close_connection(struct my_epoll_data *data) {
  if (data->ssl) {
    SSL_shutdown(data->ssl);
    SSL_free(data->ssl);
  }
  epoll_del(data);
}

int thread_func(void *args) {
  while (1) {
    struct mess m;
    msgrcv(msgid, &m, sizeof(struct my_epoll_data *), 1, 0);
    int socketfd = m.data->fd;
    int parentfd = m.data->parent_fd;
    if (socketfd == socket80fd || socketfd == socket443fd) {
      struct sockaddr saddr;
      socklen_t len;
      int cli = accept(socketfd, &saddr, &len);
      if (cli < 0) {
        epoll_mod(m.data);
        continue;
      }
      epoll_add(cli, socketfd, NULL);
    } else {
      char buff[0x1000] = {};
      if (!m.data->ssl && parentfd == socket443fd) {
        Assert((m.data->ssl = SSL_new(ctx)) != NULL, "ssl new error");
        SSL_set_accept_state(m.data->ssl);
        SSL_set_fd(m.data->ssl, socketfd);
        if (SSL_accept(m.data->ssl) != 1) {
          close_connection(m.data);
          printf("close because ssl error\n");
          continue;
        }
      }

      int n = SSL_recv(m.data->ssl, socketfd, buff, 0x1000 - 1);
      Assert(n < 0x1000 - 1, "buffer is too small");
      if (n <= 0) {
        close_connection(m.data);
        printf("close because n <= 0\n");
        continue;
      }
      char *ptr = NULL;
      printf("buff:\n%s\n", buff);

      char * h_s = strtok_r(buff, "\r\n", &ptr);
      char * host_s = strtok_r(NULL, "\r\n", &ptr);
      char * user_agent_s = strtok_r(NULL, "\r\n", &ptr);
      char * accept_encoding_s = strtok_r(NULL, "\r\n", &ptr);
      char * accept_s = strtok_r(NULL, "\r\n", &ptr);
      char * connection_s = strtok_r(NULL, "\r\n", &ptr);
      char * range_s = strtok_r(NULL, "\r\n", &ptr);
      
      
      if(strstr(accept_s, "Host") != NULL){
        printf("accept_s:\n %s\n", accept_s);
        host_s = accept_s;
      }

      char *filename = get_filename(h_s);
      char *host = get_host(host_s);
      printf("filename:\n %s\n", filename);
      printf("host:\n %s\n", host);
      if (filename == NULL) {
        send_404status(m.data->ssl, socketfd);
        close_connection(m.data);
        continue;
      }
	
      
      if(parentfd == socket80fd){
        redirect(m.data->ssl, socketfd, filename, host, range_s);
        close_connection(m.data);
        continue;
      }

      if (parentfd == socket443fd  && send_file(m.data->ssl, socketfd, filename, range_s) == -1 ) {
        //send_file(m.data->ssl, socketfd, filename, range_s);
        close_connection(m.data);
        printf("主动关闭连接\n");
        continue;
      }
    }
    epoll_mod(m.data);
  }
  return 0;
}
