/*
 * web_app.c
 *
 *  Created on: 18 но€б. 2018 г.
 *      Author: lvr_laptop
 */

#include "../global_incl.h"

#define H_RESP(...)			mg_printf(__VA_ARGS__)

const static char *TAG = "WEB_CORE";
char* doorState;
struct DoorProf_s doorWeb;

extern const char *doorPage;
extern QueueHandle_t xMailboxIndoor;
extern QueueHandle_t xMailboxOutdoor;

extern bool IsReqOpen(struct mg_str* qstr);

static void mg_ev_handler(struct mg_connection *nc, int ev, void *p) {

  switch (ev) {
  case MG_EV_ACCEPT: {
	  char addr[32];
      mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                          MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
      ESP_LOGI(TAG, "Connection %p from %s\n", nc, addr);
      break;
    }
    case MG_EV_HTTP_REQUEST: {
      char addr[32];
      struct http_message *hm = (struct http_message *) p;
      mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                          MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
      ESP_LOGI(TAG, "HTTP request from [%s]: [%.*s] [%.*s]\n[%.*s]", addr, (int) hm->method.len,
             hm->method.p, (int) hm->uri.len, hm->uri.p, (int) hm->query_string.len, hm->query_string.p);

      xQueuePeek(xMailboxOutdoor, &doorWeb, 200);
      if (doorWeb.doorOpen == STATE_OPEN) {doorState = "close";}
      else			      	 			  {doorState = "open";}

      H_RESP(nc, doorPage, doorWeb.batLevel, doorState);
      nc->flags |= MG_F_SEND_AND_CLOSE;

      if (hm->query_string.len)
      {
    	  if (IsReqOpen(&hm->query_string))
    	  {
    		  doorWeb.drCmdCode = 'O';
    		  xQueueOverwrite(xMailboxIndoor, &doorWeb);
    	  }
      }

      break;
    }

    case MG_EV_CLOSE: {
    	ESP_LOGI(TAG, "Connection %p closed\n", nc);
      break;
    }

  }
}

void WebAppCore0(void *pvParameter)
{
	wifi_init_sta();

	/* Starting Mongoose */
	struct mg_mgr mgr;
	struct mg_connection *nc;

	ESP_LOGI(TAG, "Starting web-server on port %s\n", MG_LISTEN_ADDR);

	mg_mgr_init(&mgr, NULL);

	nc = mg_bind(&mgr, MG_LISTEN_ADDR, mg_ev_handler);
	if (nc == NULL) {
		ESP_LOGI(TAG, "Error setting up listener!\n");
		return;
	}
	mg_set_protocol_http_websocket(nc);

	memset(&doorWeb, 0, sizeof(struct DoorProf_s));

	/* Processing events */
	while (1)
	{
		mg_mgr_poll(&mgr, 1000);
	}
}
