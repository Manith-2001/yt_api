#include "ANSI_colors.h"
#include "stdbool.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool check_yt_dlp() {
  int status = system("command -v yt-dlp > /dev/null 2>&1");

  if (status == 0) {
    printf(ANSI_COLOR_GREEN "yt-dlp is installed." ANSI_RESET "\n");
  } else {
    printf(ANSI_COLOR_RED "yt-dlp is NOT installed." ANSI_RESET "\n");
    return false;
  }
  return true;
}

char *id_extractor(char link[]) {
  char *id = malloc(12 * sizeof(char));
  int start, end;
  for (int i = 0; i < strlen(link); i++) {
    if (link[i] == '=') {
      start = i + 1;
      break;
    }
  }
  strncpy(id, link + start, 11);
  id[12] = '\0';
  printf("ID is : %s\n", id);
  return id;
}

char *video_exists(char *id) {
  DIR *dir = opendir("tmp");
  if (dir == NULL) {
    printf("No files downloaded as of yet\n");
    return NULL;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    printf("File : %s and ID : %s\n", entry->d_name, id);
    int result = strcmp(entry->d_name, id);
    if (result == 0) {
      printf("File found\n");
      char *path;
      asprintf(&path, "./tmp/%s", entry->d_name);
      return path;
    }
  }
  return NULL;
}
