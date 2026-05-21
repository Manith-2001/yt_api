// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved
//
// HTTP server example. This server serves both static and dynamic content.
// It opens two ports: plain HTTP on port 8000 and HTTP on port 8443.
// It implements the following endpoints:
//    /api/stats - respond with free-formatted stats on current connections
//    /api/f2/:id - wildcard example, respond with JSON string {"result": "URI"}
//    any other URI serves static files from s_root_dir
//
// To enable SSL/TLS (using self-signed certificates in PEM files),
//    1. See https://mongoose.ws/tutorials/tls/#how-to-build
//    2. curl -k https://127.0.0.1:8443

#include "mongoose/mongoose.h"
#include "router.h"
#include "threads/downloader_thread.h"
#include "threads/job_queue.h"
#include "utils/utils.h"
#include <pthread.h>

const struct mg_mem_file mg_packed_files[] = {{NULL, NULL, 0}};
static const char *s_http_addr = "http://0.0.0.0:8000";   // HTTP port
static const char *s_https_addr = "https://0.0.0.0:8443"; // HTTPS port
static const char *s_root_dir = ".";

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
