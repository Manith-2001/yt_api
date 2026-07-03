#include "../mongoose/mongoose.h"
#include "../threads/api_thread.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void data_handler(struct mg_connection *c, struct mg_http_message *hm) {
  char *link = mg_json_get_str(hm->body, "$.link");
  char *fmt = mg_json_get_str(hm->body, "$.format");

  if (link == NULL) {
    mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n",
                  "{\"error\": \"missing 'link' field\"}\n");
    return;
  }
  if (fmt == NULL) {
    fmt = "webm";
  }
  void *fd = c->fd;
  c->fd = (void *)MG_INVALID_SOCKET;
  c->is_closing = 1;
  WorkerArgs *args = calloc(1, sizeof(WorkerArgs));
  args->fd = fd;
  args->link = strdup(link);
  args->fmt = fmt;
  pthread_t t;
  pthread_create(&t, NULL, api_function, args);
  pthread_detach(t);
  return;
}
