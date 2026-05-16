#include "api_thread.h"
#include "job_queue.h"
#include <_time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_pthread/_pthread_t.h>
#include <sys/socket.h>
#include <unistd.h>

void *api_function(void *arg) {
  WorkerArgs *args = arg;
  pthread_t tid = pthread_self();
  printf("Spun api thread of id : %d", *(int *)tid);
  int sockfd = (int)(intptr_t)args->fd;
  enque_download(args->link, *(int *)tid);
  // Send headers once — no Content-Length, chunked encoding
  const char *headers = "HTTP/1.1 200 OK\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "Content-Type: video/mp4\r\n"
                        "Connection: close\r\n\r\n";
  send(sockfd, headers, strlen(headers), 0);

  // Keep sending chunks until client disconnects
  char *json_body =
      "{\"status\": \"success\", \"message\": \"Hello from C API\"}";
  while (1) {
    char chunk[1024];
    int chunk_len = snprintf(chunk, sizeof(chunk),
                             "%zx\r\n%s\r\n", // hex size, CRLF, data, CRLF
                             strlen(json_body), json_body);

    int n = send(sockfd, chunk, chunk_len, MSG_NOSIGNAL);
    if (n <= 0) {
      printf("client gone\n");
      break; // client gone
    }
    completed_msg c_msg = dequeu_completed();
    printf("Got link : %s of id : %d\n", c_msg.link, c_msg.tid);
    if (c_msg.tid == *(int *)tid) {
      printf("my link breaking\n");
      // free(c_msg.link);
      break;
    } else {
      printf("not myu link queing again\n");
      enque_completed(c_msg.link, c_msg.tid);
    }
    sleep(1);
  }

  close(sockfd);
  free(args->link);
  free(args);
  return NULL;
}
