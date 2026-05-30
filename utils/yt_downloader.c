#include "yt_downloader.h"
#include "../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *yt_download(char *link) {
  char *yt_id = id_extractor(link);
  char *yt_path = video_exists(yt_id);
  free(yt_id);
  if (yt_path != NULL) {
    return strdup(yt_path);
  }
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
  printf("Path is : %s\n", path);

  sprintf(command,
          "yt-dlp -S \"height:720\" -P ./tmp/ -o \"%%(id)s.%%(ext)s\" \"%s\"",
          link);
  system(command);
  return strdup(path);
}
