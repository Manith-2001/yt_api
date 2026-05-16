#include "job_queue.h"
#include <_string.h>
#include <stdio.h>
#include <string.h>
#include <sys/_pthread/_pthread_t.h>
#include <sys/fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

key_t download_key;
int downloadqueue_id;
key_t complete_key;
int completequeue_id;

void queue_init(void) {

  close(open("downloadqueue", O_CREAT | O_WRONLY, 0644));
  close(open("completedqueue", O_CREAT | O_WRONLY, 0644));
  download_key = ftok("downloadqueue", 65);
  downloadqueue_id = msgget(download_key, 0666 | IPC_CREAT);
  complete_key = ftok("completedqueue", 65);
  completequeue_id = msgget(complete_key, 0666 | IPC_CREAT);
}

void enque_download(char *link, int tid) {
  download_msg d_msg;
  d_msg.mtype = 1;
  size_t len = strlen(link);
  strcpy(d_msg.link, link);
  d_msg.tid = tid;
  if (msgsnd(downloadqueue_id, &d_msg, sizeof(download_msg) - sizeof(long),
             0) == -1) {
    // TDOO : handle error sending message
    printf("Error sending message to download queue\n");
  }
}

download_msg dequeu_download(void) {
  download_msg r_msg;
  if (msgrcv(downloadqueue_id, &r_msg, sizeof(download_msg) - sizeof(long), 1,
             0) == -1) {
    perror("msgrcv");
  }
  printf("Download Message received: %s\n", r_msg.link);
  printf("Download ID received: %d\n", r_msg.tid);
  return r_msg;
}

void enque_completed(char *link, int tid) {
  completed_msg c_msg;
  c_msg.mtype = 1;
  size_t len = strlen(link);
  strcpy(c_msg.link, link);
  c_msg.tid = tid;
  if (msgsnd(completequeue_id, &c_msg, sizeof(completed_msg) - sizeof(long),
             0) == -1) {
    // TDOO : handle error sending message
    printf("Error sending message to download queue\n");
  }
}

completed_msg dequeu_completed(void) {
  completed_msg r_msg;
  if (msgrcv(completequeue_id, &r_msg, sizeof(completed_msg) - sizeof(long), 1,
             0) == -1) {
    perror("msgrcv");
  }
  printf("Completed Message received: %s\n", r_msg.link);
  return r_msg;
}
