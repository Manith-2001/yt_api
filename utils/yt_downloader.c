#include "yt_downloader.h"
#include <_stdio.h>
#include <_string.h>
#include <stdio.h>
#include <stdlib.h>

char *yt_download(char *link) {
  char command[1536];
  sprintf(command,
          "yt-dlp -P ./tmp/ -o \"%%(id)s.%%(ext)s\" --print filename \"%s\"",
          link);
  FILE *fp = popen(command, "r");
  char path[1024];
  fgets(path, sizeof(path), fp);
  pclose(fp);
  int len = strlen(path);
  if (len > 0 && path[len - 1] == '\n') {
    path[len - 1] = '\0';
  }
  printf("Path is : %s", path);

  sprintf(command, "yt-dlp -P ./tmp/ -o \"%%(id)s.%%(ext)s\" \"%s\"", link);
  system(command);
  return strdup(path);
}
