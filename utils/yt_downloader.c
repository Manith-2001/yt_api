#include <stdio.h>
#include <stdlib.h>

void yt_download(char *link) {
  char command[1056];
  sprintf(command, "yt-dlp -P ./tmp/ \"%s\"", link);
  system(command);
}
