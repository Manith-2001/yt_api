#pragma once

typedef struct {
  void *fd;   // the hijacked socket
  char *link; // the youtube URL to download
  char *fmt;  // format in which it should be downloaded
} WorkerArgs;

void *api_function(void *arg);
