#include "../mongoose/mongoose.h"

void data_handler(struct mg_connection *c, struct mg_http_message *hm) {
  mg_http_reply(c, 200, "", "{\"received\": \"%.*s\"}\n", (int)hm->body.len,
                hm->body.buf);
  return;
}
