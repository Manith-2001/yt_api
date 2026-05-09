#pragma once

typedef struct {
  void *fd;        // the hijacked socket
  char* link; // the youtube URL to download
} WorkerArgs;

void *api_function(void *arg);
