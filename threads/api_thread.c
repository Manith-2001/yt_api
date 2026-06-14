#include "api_thread.h"
#include "job_queue.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

void *api_function(void *arg) {
  WorkerArgs *args = arg;
  pthread_t tid = pthread_self();
  printf("Spun api thread of id : %d\n", *(int *)tid);
  int sockfd = (int)(intptr_t)args->fd;
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK);
  enque_download(args->link, args->fmt, *(int *)tid);
  // Send headers once — no Content-Length, chunked encoding
  const char *headers = "HTTP/1.1 200 OK\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "Content-Type: application/octet-stream\r\n"
                        "Connection: close\r\n\r\n";
  send(sockfd, headers, strlen(headers), 0);

  while (1) {
    completed_msg c_msg = dequeu_completed();
    printf("Got link : %s of id : %d\n", c_msg.link, c_msg.tid);
    if (c_msg.tid == *(int *)tid) {
      FILE *f = fopen(c_msg.link, "rb");
      if (f) {
        printf("Streaming Downloaded file\n");
        char buf[8192];
        int n;
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
          char chunk_header[32];
          int hdr_len =
              snprintf(chunk_header, sizeof(chunk_header), "%x\r\n", n);
          send(sockfd, chunk_header, hdr_len, MSG_NOSIGNAL);
          send(sockfd, buf, n, MSG_NOSIGNAL);
          send(sockfd, "\r\n", 2, MSG_NOSIGNAL);
        }
        fclose(f);
        send(sockfd, "0\r\n\r\n", 5, MSG_NOSIGNAL);
        shutdown(sockfd, SHUT_WR);
      } else {
        printf("Streaming error \n");
        perror("Error opening file");
        send(sockfd, "0\r\n\r\n", 5, MSG_NOSIGNAL);
      }
      break;
    } else {
      printf("not myu link queing again\n");
      enque_completed(c_msg.link, c_msg.tid);
    }
    sleep(1);
  }

  char drain[1024];
  while (recv(sockfd, drain, sizeof(drain), 0) > 0) {
  }
  close(sockfd);
  free(args->link);
  free(args);
  return NULL;
}
