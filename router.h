#pragma once
#include "mongoose/mongoose.h"

typedef struct route route;
typedef void (*router_handler_t)(struct mg_connection *c,
                                 struct mg_http_message *hm);
struct route {
  const char *uri;
  const char *method;
  router_handler_t handler;
};

void dispatch(struct mg_connection *c, struct mg_http_message *hm);
