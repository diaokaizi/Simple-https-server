#include "general.h"

int redirect(SSL *ssl, int socketfd, char * filename, char * host , char * range) {
  char head_buff[512] = "HTTP/1.1 301 Moved Permanently\r\n";
  strcat(head_buff, "Location: https://");
  strcat(host, filename);
  strcat(head_buff, host);
  strcat(head_buff, "\r\n");
  if (range != NULL) {
       strcat(head_buff, range);
       strcat(head_buff, "\r\n");
  }
  SSL_send(ssl, socketfd, head_buff, strlen(head_buff));
  return 0;
}

