#include "downloader_thread.h"
#include "../utils/yt_downloader.h"
#include "job_queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_t download_thread;
int id1 = 1;

void download_thread_init(void) {
  if (pthread_create(&download_thread, NULL, download_function, &id1)) {
    fprintf(stderr, "Error creating thread 1\n");
    return;
  }
}

void *download_function(void *arg) {
  int thread_id = *(int *)arg; // Cast the argument to an integer
  printf("Thread %d is running\n", thread_id);
  while (1) {
    char *link = dequeu_download();
    if (link) {
      yt_download(link);
      free(link);
    }
  }
}
