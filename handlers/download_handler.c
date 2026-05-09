#include "../mongoose/mongoose.h"
#include "../threads/api_thread.h"
#include "../threads/job_queue.h"
#include <_string.h>
#include <_time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void data_handler(struct mg_connection *c, struct mg_http_message *hm) {
  char *link = mg_json_get_str(hm->body, "$.link");

  if (link == NULL) {
    mg_http_reply(c, 400, "", "{\"error\": \"missing 'link' field\"}\n");
    return;
  }
  // enque_download(link);
  void *fd = c->fd;
  c->fd = (void *)MG_INVALID_SOCKET;
  c->is_closing = 1;
  WorkerArgs *args = calloc(1, sizeof(WorkerArgs));
  args->fd = fd;
  args->link = link;
  pthread_t t;
  pthread_create(&t, NULL, api_function, args);
  pthread_detach(t);
  // mg_http_reply(c, 200, "", "{\"received\": \"%s\"}\n", link);
  return;
}
