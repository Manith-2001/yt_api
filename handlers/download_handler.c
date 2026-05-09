#include "../mongoose/mongoose.h"
#include "../threads/job_queue.h"
#include <stdio.h>

void data_handler(struct mg_connection *c, struct mg_http_message *hm) {
  char *link = mg_json_get_str(hm->body, "$.link");

  if (link == NULL) {
    mg_http_reply(c, 400, "", "{\"error\": \"missing 'link' field\"}\n");
    return;
  }
  enque_download(link);

  mg_http_reply(c, 200, "", "{\"received\": \"%s\"}\n", link);
  mg_free(link);
  return;
}
