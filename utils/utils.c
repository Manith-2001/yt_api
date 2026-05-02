#include "ANSI_colors.h"
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>

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
