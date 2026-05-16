#include <stdio.h>
#include <stdlib.h>
#include <_string.h>
#include "yt_downloader.h"

char *yt_download(char *link) {
  char command[1536];
  sprintf(command,
          "yt-dlp -P ./tmp/ -o \"%%(id)s.%%(ext)s\" --print filename \"%s\"",
          link);
  FILE *fp = popen(command, "r");
  char path[1024];
  fgets(path, sizeof(path), fp);
  printf("Path is : %s", path);

  sprintf(command, "yt-dlp -P ./tmp/ -o \"%%(id)s.%%(ext)s\" \"%s\"", link);
  system(command);
  return strdup(path);
}
