#include "job_queue.h"
#include <_string.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

key_t download_key;
int downloadqueue_id;
key_t complete_key;
int completequeue_id;

void queue_init(void) {
  download_key = ftok("downloadqueue", 65);
  downloadqueue_id = msgget(download_key, 0666 | IPC_CREAT);
}

void enque_download(char *link) {
  download_msg d_msg;
  d_msg.mtype = 1;
  size_t len = strlen(link);
  strcpy(d_msg.link, link);
  if (msgsnd(downloadqueue_id, &d_msg, sizeof(d_msg.link), 0) == -1) {
    // TDOO : handle error sending message
    printf("Error sending message to download queue\n");
  }
}

char *dequeu_download(void) {
  download_msg r_msg;
  if (msgrcv(downloadqueue_id, &r_msg, sizeof(r_msg.link), 1, 0) == -1) {
    perror("msgrcv");
  }
  printf("Message received: %s\n", r_msg.link);
  return strdup(r_msg.link);
}
