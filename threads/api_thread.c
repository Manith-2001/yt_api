#include "api_thread.h"
#include <_time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void *api_function(void *arg) {
  WorkerArgs *args = arg;
  int sockfd = (int)(intptr_t)args->fd;

  // Send headers once — no Content-Length, chunked encoding
  const char *headers = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "Connection: keep-alive\r\n"
                        "\r\n";
  send(sockfd, headers, strlen(headers), MSG_NOSIGNAL);

  // Keep sending chunks until client disconnects
  char *json_body =
      "{\"status\": \"success\", \"message\": \"Hello from C API\"}";
  while (1) {
    char chunk[1024];
    int chunk_len = snprintf(chunk, sizeof(chunk),
                             "%zx\r\n%s\r\n", // hex size, CRLF, data, CRLF
                             strlen(json_body), json_body);

    int n = send(sockfd, chunk, chunk_len, MSG_NOSIGNAL);
    if (n <= 0)
      break; // client gone
    sleep(1);
  }

  close(sockfd);
  free(args->link);
  free(args);
  return NULL;
}
