#include "mongoose/mongoose.h"
#include "router.h"
#include "threads/downloader_thread.h"
#include "threads/job_queue.h"
#include "utils/utils.h"
#include <pthread.h>
#include <sys/stat.h>

const struct mg_mem_file mg_packed_files[] = {{NULL, NULL, 0}};
static const char *s_http_addr = "http://0.0.0.0:8000";   // HTTP port
static const char *s_https_addr = "https://0.0.0.0:8443"; // HTTPS port (for future implementation)

static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    dispatch(c, (struct mg_http_message *)ev_data);
  }
}

int main(void) {
  if (!check_yt_dlp()) {
    return 0;
  }
  queue_init();
  mkdir("tmp", 0755);
  download_thread_init();
  struct mg_mgr mgr;                            // Event manager
  mg_log_set(MG_LL_DEBUG);                      // Set log level
  mg_mgr_init(&mgr);                            // Initialise event manager
  mg_http_listen(&mgr, s_http_addr, fn, NULL);  // Create HTTP listener
  for (;;)
    mg_mgr_poll(&mgr, 1000); // Infinite event loop
  mg_mgr_free(&mgr);
  return 0;
}
