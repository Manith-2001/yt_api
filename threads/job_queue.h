#pragma once

#define MAX_LINK_LEN 1024

typedef struct {
  long mtype;
  char link[MAX_LINK_LEN];
} download_msg;

void queue_init(void);
void enque_download(char *link);
char *dequeu_download(void);
