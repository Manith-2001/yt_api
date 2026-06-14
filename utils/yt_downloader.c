#include "yt_downloader.h"
#include "../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *yt_download(char *link, char *fmt) {
  char *yt_id = id_extractor(link);
  char *file_name = strcat(yt_id, ".");
  file_name = strcat(file_name, fmt);
  char *yt_path = video_exists(file_name);
  free(yt_id);
  if (yt_path != NULL) {
    return strdup(yt_path);
  }
  char command[1536];
  sprintf(command,
          "yt-dlp --merge-output-format %s -P ./tmp/ -o \"%%(id)s.%%(ext)s\" "
          "--print filename \"%s\"",
          fmt, link);
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
          "yt-dlp --merge-output-format %s -S \"height:720\" -P ./tmp/ -o "
          "\"%%(id)s.%%(ext)s\" \"%s\"",
          fmt, link);
  system(command);
  return strdup(path);
}
