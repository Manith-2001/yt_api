#include "router.h"
#include "handlers/download_handler.h"
#include "mongoose/mongoose.h"

static const route s_routes[] = {{"/api/download", "POST", data_handler}};

void dispatch(struct mg_connection *c, struct mg_http_message *hm) {
  for (size_t i = 0; i < sizeof(s_routes) / sizeof(s_routes[0]); i++) {
    const route *r = &s_routes[i];
    if (!mg_match(hm->uri, mg_str(r->uri), NULL))
      continue;
    if (mg_match(hm->uri, mg_str(r->uri), NULL)) {
      if (!mg_match(hm->method, mg_str(r->method), NULL)) {
        mg_http_reply(c, 405, "", "{\"error\": \"method not allowed\"}\n");
        return;
      }
    }
    r->handler(c, hm);
    return;
  }

  mg_http_reply(c, 404, "", "{Path Not found}\n", 0, NULL);
}
