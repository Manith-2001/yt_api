#pragma once

#define MAX_LINK_LEN 1024
#define MAX_FMT_LEN 10

typedef struct {
  long mtype;
  char link[MAX_LINK_LEN];
  char fmt[MAX_FMT_LEN];
  int tid;
} download_msg;

typedef struct {
  long mtype;
  char link[MAX_LINK_LEN];
  int tid;
} completed_msg;

void queue_init(void);
void enque_download(char *link, char *fmt, int tid);
download_msg dequeu_download(void);
void enque_completed(char *link, int tid);
completed_msg dequeu_completed(void);
